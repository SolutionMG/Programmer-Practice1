#include "pch.h"
#include "BaseServer.h"
#include "ChattingRoom.h"
#include "PlayerInfo.h"

#include <algorithm>
#include <chrono>
#include <thread>
#include <ctime>

/// Static 변수
BaseServer::BaseServer() : m_iocpHandle(NULL), m_listenSocket(INVALID_SOCKET)
{   
}

BaseServer::~BaseServer() noexcept = default;

bool BaseServer::Initialize()
{
    ///Initialize
    m_players.reserve(100);
    for (int i = 0; i < 1000; ++i) //방번호
        m_charRoomNumber.push(i);

    InitializeCommandFunction();

    ///한국어 출력
    std::wcout.imbue(std::locale("korean"));

    WSADATA wsaData;
    int returnValue = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (returnValue != 0)
    {
        DisplayError("WSAStartup Initialize()");
        return false;
    }

    m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    if (m_listenSocket == INVALID_SOCKET)
    {
        DisplayError("WSASocket Initialize()");
        return false;
    }

    ///Nagle Off Option
    int socketOption = 1;
    returnValue = setsockopt(m_listenSocket, SOL_SOCKET, TCP_NODELAY, (const char*)(&socketOption), sizeof(socketOption));
    if (returnValue != 0)
    {
        DisplayError("setsockopt Initialize()");
        return false;
    }

    std::cout << "Server Initialize Sueccess..." << std::endl;
    return true;
}

bool BaseServer::OpenServer()
{
    /// IOCP 객체 생성
    m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_listenSocket), m_iocpHandle, 0, 0);

    WSAOVERLAPPED_EXTEND over;
    Accept(&over);

    ///WorkerThread 생성 - Accept, Send, Recv 기능 수행 IOCP 쓰레드 풀을 통해 TOTALCORE/2 개수의 스레드로 동작
    std::vector<std::thread> workerThreads;
    for (int i = 0; i < (InitailizeServer::TOTALCORE / 2); ++i)
    {
        workerThreads.emplace_back([&]() {MainWorkProcess(); });
    }

    ///Other Thread...

    ///LogOnThread - 로그인 관리 Thread
    std::thread logonThread{ [&]() {LogOnCommandProcess();} };
    logonThread.join();

    for (auto& wthread : workerThreads)
    {
        wthread.join();
    }

    closesocket(m_listenSocket);
    WSACleanup();

    return false;
}

bool BaseServer::InitializeCommandFunction()
{
    m_commandFunctions.reserve(10);
    m_commandFunctions.insert({ CommandMessage::COMMANDLIST, [&](const SOCKET& socket) { ReqeustCommandList(socket);} });
    m_commandFunctions.insert({ CommandMessage::USERLIST, [&](const SOCKET& socket) { RequestUserList(socket); } });
    m_commandFunctions.insert({ CommandMessage::EXIT, [&](const SOCKET& socket) { RequestExit(socket); } });
    m_commandFunctions.insert({ CommandMessage::ROOMCREATE, [&](const SOCKET& socket) { RequestRoomCreate(socket); } });
    m_commandFunctions.insert({ CommandMessage::ROOMLIST, [&](const SOCKET& socket) { RequestRoomList(socket); } });
    m_commandFunctions.insert({ CommandMessage::ROOMENTER, [&](const SOCKET& socket) { RequestRoomEnter(socket); } });
    m_commandFunctions.insert({ CommandMessage::PLAYERINFO, [&](const SOCKET& socket) { RequestUserInfo(socket); } });
    m_commandFunctions.insert({ CommandMessage::ROOMINFO, [&](const SOCKET& socket) { RequestRoomInfo(socket); } });
    
    return true;
}

bool BaseServer::MainWorkProcess()
{
    while (true)
    {
        DWORD bytes;
        ULONG_PTR completionKey;
        WSAOVERLAPPED* over;

        bool returnValue = GetQueuedCompletionStatus(m_iocpHandle, &bytes, &completionKey, &over, INFINITE);
        
        SOCKET userKey = static_cast<SOCKET> (completionKey);
        WSAOVERLAPPED_EXTEND* overExtend = reinterpret_cast<WSAOVERLAPPED_EXTEND*>(over);

        switch (overExtend->opType)
        {
        case EOperationType::RECV:
        {
            if (bytes == 0 || returnValue == false)
            {
                Disconnect(userKey);
                break;
            }
            ReassemblePacket(overExtend->networkBuffer, bytes, userKey);
        }
        break;

        case EOperationType::SEND:
        {
            if (bytes == 0 || returnValue == false) {
                Disconnect(userKey);
                break;
            }
            delete over;
            over = nullptr;
        }
        break;

        case EOperationType::ACCEPT:
        {
            if (returnValue == false)
            {

            }

            userKey = overExtend->socket;
            std::cout << "ACCEPT Player [" << userKey << "]" << std::endl;
            AddNewClient(userKey);
            m_playersLock.lock();
            m_players[userKey].ReceivePacket();
            m_playersLock.unlock();

            Accept(overExtend);
        }
        break;
        }
    }
    return false;
}

bool BaseServer::Listen()
{
    SOCKADDR_IN serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(InitailizeServer::SERVERPORT);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    int returnValue = -1;
    returnValue = bind(m_listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));

    if (returnValue != 0)
    {
        DisplayError("bind Listen()");
        return false;
    }

    returnValue = listen(m_listenSocket, SOMAXCONN);
    if (returnValue != 0)
    {
        DisplayError("listen Listen()");
        return false;
    }

    std::cout << "Waiting For Player..." << std::endl;
    return true;
}

bool BaseServer::Accept(WSAOVERLAPPED_EXTEND* over)
{
    SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socket == INVALID_SOCKET)
    {
        DisplayError("WSAStartup Accept()");
        return false;
    }

    ///Nagle Off Option
    int socketOption = 1;
    int returnValue = setsockopt(socket, SOL_SOCKET, TCP_NODELAY, (const char*)(&socketOption), sizeof(socketOption));
    if (returnValue != 0)
    {
        DisplayError("setsockopt Accept()");
        return false;
    }

    ZeroMemory(&over->over, sizeof(over->over));

    DWORD bytes;
    over->opType = EOperationType::ACCEPT;
    over->socket = socket;

    bool returnValue2 = AcceptEx(m_listenSocket, socket, over->networkBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, &over->over);
    if (returnValue2 == false)
    {
        ///작업이 성공적으로 시작, 아직 진행 중
        if (WSAGetLastError() == ERROR_IO_PENDING)
            return true;

        DisplayError("AcceptEx Accept()");
        return false;
    }

    return true;
}

bool BaseServer::AddNewClient(const SOCKET& socket)
{
    m_playersLock.lock();
    m_players[socket].StartLock();
    m_players[socket].SetSocket(socket);
    m_players[socket].SetOverlappedOperation(EOperationType::RECV);
    m_players[socket].SetState(ClientState::ACCESS);
    m_players[socket].EndLock();
    m_playersLock.unlock();

    HANDLE returnValue2 = CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), m_iocpHandle, socket, 0);
    if (returnValue2 == NULL) {
        DisplayError("CreateIoCompletionPort AddNewClient()");
        return false;
    }

    /// 로그인 요청 메시지 전송
    m_playersLock.lock();
    m_players[socket].SendPacket(RenderMessageMacro::ACCESSMESSAGE, sizeof(RenderMessageMacro::ACCESSMESSAGE));
    m_players[socket].SendPacket(RenderMessageMacro::LOGONREQUEST, sizeof(RenderMessageMacro::LOGONREQUEST));
    m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
    m_playersLock.unlock();

    return true;
}

bool BaseServer::StateWorkBranch(const SOCKET& socket, const std::string_view& command)
{
    m_playersLock.lock();
    m_players[socket].StartLock();
    switch (m_players[socket].GetState())
    {
    case ClientState::ACCESS:
    {
        m_players[socket].EndLock();

        if (command.length() < sizeof(CommandMessage::LOGON))
        {
            m_players[socket].StartLock();
            m_players[socket].ClearChattingBuffer();
            m_players[socket].EndLock();

            m_players[socket].SendPacket(RenderMessageMacro::LOGONREQUEST, sizeof(RenderMessageMacro::LOGONREQUEST));
            m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
            m_players[socket].ReceivePacket();
            m_playersLock.unlock();
            break;
        }
        m_playersLock.unlock();

        std::string_view checkCommand = { command.cbegin(), command.cbegin() + sizeof(CommandMessage::LOGON) - 1 };
        if (checkCommand == CommandMessage::LOGON)
        {
            m_logOn.push(socket);
        }
        else
        {
            m_playersLock.lock();
            m_players[socket].StartLock();
            m_players[socket].ClearChattingBuffer();
            m_players[socket].EndLock();
            m_players[socket].SendPacket(RenderMessageMacro::LOGONREQUEST, sizeof(RenderMessageMacro::LOGONREQUEST));
            m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
            m_players[socket].ReceivePacket();
            m_playersLock.unlock();

        }
    }
    break;
    case ClientState::LOGON:
    {
        m_players[socket].EndLock();
        m_playersLock.unlock();
        CommandWorkBranch(socket, command);
    }
    break;
    case ClientState::ROOM:
    {
        m_players[socket].EndLock();
        m_playersLock.unlock();

        Chatting(socket);
        m_playersLock.lock();
        m_players[socket].StartLock();
        m_players[socket].ClearChattingBuffer();
        m_players[socket].EndLock();

        m_players[socket].ReceivePacket();
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_playersLock.unlock();
    }
    break;
    case ClientState::END:
    {
        m_players[socket].ClearChattingBuffer();
        m_players[socket].EndLock();
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
    }
    break;
    default:
    {
        m_players[socket].ClearChattingBuffer();
        m_players[socket].EndLock();
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
    }
    break;
    }

    return true;
}

bool BaseServer::CommandWorkBranch(const SOCKET& socket, const std::string_view& request)
{
    if (request.empty())
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return false;
    }

    bool flag = true;
    if (request.length() == 1)
    {
        std::string_view checkCommand1Word = { request.cbegin(), request.cbegin() + 1 };
        
        auto iterator = m_commandFunctions.find(checkCommand1Word);
        if (iterator == m_commandFunctions.end())
        {
            m_playersLock.lock();
            m_players[socket].StartLock();
            m_players[socket].ClearChattingBuffer();
            m_players[socket].EndLock();
            m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
            m_players[socket].ReceivePacket();
            m_playersLock.unlock();
            flag = false;
        }
        else
        {
            if (flag == true)
            {
                m_commandFunctions[checkCommand1Word](socket);
                m_playersLock.lock();
                m_players[socket].StartLock();
                m_players[socket].ClearChattingBuffer();
                m_players[socket].EndLock();
                m_playersLock.unlock();
            }
            return true;
        }
    }
    else if (request.length() >= 2) 
    {
        std::string_view checkCommand2Word = "";
        if (request[1] == ' ')
        {
            checkCommand2Word = { request.cbegin(), request.cbegin() + 1 };
        }
        else
        {
            checkCommand2Word = { request.cbegin(), request.cbegin() + 2 };
        }
        auto iterator = m_commandFunctions.find(checkCommand2Word);
        if (iterator == m_commandFunctions.end())
        {
            m_playersLock.lock();
            m_players[socket].StartLock();
            m_players[socket].ClearChattingBuffer();
            m_players[socket].EndLock();
            m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
            m_players[socket].ReceivePacket();
            m_playersLock.unlock();
            flag = false;
        }
        if (flag == true)
        {
            m_commandFunctions[checkCommand2Word](socket);
            m_playersLock.lock();
            m_players[socket].StartLock();
            m_players[socket].ClearChattingBuffer();
            m_players[socket].EndLock();
            m_playersLock.unlock();
        }
        return true;
    }
    return false;
}

bool BaseServer::ReqeustCommandList(const SOCKET& socket)
{
    m_playersLock.lock();
    m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
    m_players[socket].SendPacket(RenderMessageMacro::GUIDEMESSAGE, sizeof(RenderMessageMacro::GUIDEMESSAGE));
    m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
    m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
    m_players[socket].ReceivePacket();
    m_playersLock.unlock();
    return true;
}

bool BaseServer::RequestExit(const SOCKET& socket)
{
    return Disconnect(socket);
}

bool BaseServer::RequestUserInfo(const SOCKET& socket)
{
    m_playersLock.lock();
    m_players[socket].StartLock();
    std::string_view message = { m_players[socket].GetChattingLog().cbegin(), m_players[socket].GetChattingLog().cend() };
    m_players[socket].EndLock();
    m_playersLock.unlock();

    if (message[2] != ' ')
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return false;
    }

    std::string_view userName = { message.cbegin() + 3, message.cend() };
    bool flag = false;
    SOCKET userSocket;

    m_playersLock.lock();
    for (const auto& user : m_players)
    {
        if (user.second.GetState() > ClientState::ACCESS && user.second.GetState() < ClientState::EXIT)
        {
            if (userName == user.second.GetName())
            {
                userSocket = user.first;
                flag = true;
                break;
            }
        }
    }
    m_playersLock.unlock();

    if (flag == true)
    {
        m_playersLock.lock();
        auto& player = m_players[userSocket];
        m_playersLock.unlock();

        std::string userInfo = { userName.cbegin(), userName.cend()};

        switch (player.GetState())
        {
        case ClientState::ACCESS:
        {
        }
        break;
        case ClientState::LOGON:
        {
            userInfo += "님은 로비에 있습니다.\n\r";
            m_playersLock.lock();
            m_players[socket].SendPacket(userInfo.c_str(), static_cast<unsigned short>(userInfo.size()));
            m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
            m_playersLock.unlock();
        }
        break;
        case ClientState::ROOM:
        {
            userInfo += "님은";
            m_playersLock.lock();
            int roomNumber = m_players[userSocket].GetRoomNumber();
            m_playersLock.unlock();
            char roomIndex[8];
            _itoa_s(roomNumber, roomIndex, 10);
            userInfo += roomIndex;
            userInfo += "번 방에 있습니다.\n\r";
            m_playersLock.lock();
            m_players[socket].SendPacket(userInfo.c_str(), static_cast<unsigned short>( userInfo.size()));
            m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
            m_playersLock.unlock();
        }
        break;
        case ClientState::EXIT:
        {
        }
        break;
        case ClientState::END:
        {
        }
        break;
        default:
            break;
        }
    }
    else
    {
        m_players[socket].SendPacket(RenderMessageMacro::USERINFOMESSAGEFAILED, sizeof(RenderMessageMacro::USERINFOMESSAGEFAILED));
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
    }
    m_players[socket].ReceivePacket();

    return true;
}

bool BaseServer::RequestUserList(const SOCKET& socket)
{
    m_playersLock.lock();
    m_players[socket].StartLock();
    std::string_view message = { m_players[socket].GetChattingLog().cbegin(), m_players[socket].GetChattingLog().cend() };
    m_players[socket].EndLock();
    m_playersLock.unlock();

    if (message.length() > 2)
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return false;
    }

    m_playersLock.lock();
    m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
    std::string userNameList = "--------------------------------유저 목록--------------------------------\n\r";

    for (const auto& player : m_players)
    {
        if ( player.second.GetState() > ClientState::ACCESS && player.second.GetState() < ClientState::EXIT )
        {
            userNameList += player.second.GetName();
            userNameList += "\n\r";
        }
    }
    m_playersLock.unlock();

    m_playersLock.lock();
    m_players[socket].SendPacket(userNameList.c_str(), static_cast<short>(userNameList.size()));
    m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
    m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
    m_players[socket].ReceivePacket();
    m_playersLock.unlock();
    return true;
}

bool BaseServer::RequestRoomInfo(const SOCKET& socket)
{
    m_playersLock.lock();
    auto& player = m_players[socket];
    m_playersLock.unlock();

    std::string_view message = { player.GetChattingLog().cbegin(), player.GetChattingLog().cend() };

    if (message[2] != ' ')
    {
        player.SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        player.ReceivePacket();
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        return false;
    }

    std::string roomIndex = { message.cbegin() + 3, message.cend() };
    int index = atoi(roomIndex.c_str());
    std::string sendMessage = "[";

    ///현재 시간
    ///입장 시간 기입
    std::string roomInTime = "";
    char timebuffer[64];
    time_t currentTime = time(NULL);
    tm currentTm;
    localtime_s(&currentTm, &currentTime);

    strftime(timebuffer, sizeof(timebuffer), "%m-%d-%Y %X", &currentTm);
    roomInTime = timebuffer;

    m_chattRoomLock.lock();
    for (const auto& room : m_chattingRooms)
    {
        if (index == room.first)
        {
            ///방 정보 메시지 보내기
            char roomTotal[8];
            char roomMax[8];
            _itoa_s(room.second.GetTotalPlayer(), roomTotal, 10);
            _itoa_s(room.second.GetMaxUser(), roomMax, 10);

            sendMessage += roomIndex;
            sendMessage += "] (";
            sendMessage += roomTotal;
            sendMessage += "/";
            sendMessage += roomMax;
            sendMessage += ") ";
            sendMessage += room.second.GetName();
            sendMessage += "\n\r";
            sendMessage += "개설시간:   ";
            sendMessage += room.second.GetRoomInTime();

            for (const auto& id : room.second.GetAccessorIndex())
            {
                m_playersLock.lock();
                sendMessage += "\n\r참여자:";
                sendMessage += m_players[id].GetName();
                sendMessage += "    참여시간:   ";
                sendMessage += m_players[id].GetRoomInTime();
                sendMessage += "\n\r";
                m_playersLock.unlock();
            }
            player.SendPacket(RenderMessageMacro::ROOMINFOLINEMESSAGE, sizeof(RenderMessageMacro::ROOMINFOLINEMESSAGE));
            player.SendPacket(sendMessage.c_str(), static_cast<unsigned short> (sendMessage.size()));
            player.SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
            
            break;
        }
    }
    m_chattRoomLock.unlock();
    player.SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
    player.ReceivePacket();
    return true;
}

bool BaseServer::RequestRoomCreate(const SOCKET& socket)
{
    ///o 최대인원 방이름
    m_playersLock.lock();
    m_players[socket].StartLock();
    std::string_view message = { m_players[socket].GetChattingLog().cbegin(), m_players[socket].GetChattingLog().cend() };
    m_players[socket].EndLock();
    m_playersLock.unlock();

    if (message.length() <= 3)
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::FAILEDCOMMANDMESSAGE, sizeof(RenderMessageMacro::FAILEDCOMMANDMESSAGE));
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return false;
    }

    int maxUser = message[2] - '0';
    /// 인원 초과
    if (maxUser < 2 || maxUser > 20)
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::CREATEROOMFAILEDOVERUSERS, sizeof(RenderMessageMacro::CREATEROOMFAILEDOVERUSERS));
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return false;
    }
    
    /// 방이름 여부 
    std::string roomName = { message.cbegin() + 4, message.cend() };
    
    m_playersLock.lock();
    for (const auto& room : m_chattingRooms)
    {
        if (strcmp(room.second.GetName(), roomName.c_str()) == 0)
        {
            m_players[socket].SendPacket(RenderMessageMacro::CREATEROOMFAILEDMESSAGE, sizeof(RenderMessageMacro::CREATEROOMFAILEDMESSAGE));
            m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
            m_players[socket].ReceivePacket();
            return false;
        }
    }
    m_playersLock.unlock();

    m_chattRoomNumLock.lock();
    int number = m_charRoomNumber.top();
    m_charRoomNumber.pop();
    m_chattRoomNumLock.unlock();

    ///입장 시간 기입
    std::string roomInTime = "";
    ///현재 시간
    char timebuffer[64];
    time_t currentTime = time(NULL);
    tm currentTm;
    localtime_s(&currentTm, &currentTime);

    strftime(timebuffer, sizeof(timebuffer), "%m-%d-%Y %X", &currentTm);
    roomInTime = timebuffer;

    m_chattRoomLock.lock();
    m_chattingRooms[number].StartLock();
    m_chattingRooms[number].SetIndex(number);
    m_chattingRooms[number].SetMaxUser(maxUser);
    m_chattingRooms[number].SetName(roomName.c_str());
    m_chattingRooms[number].SetTotalPlayers(1);
    m_chattingRooms[number].PushAccessor(socket);
    m_chattingRooms[number].EndLock();
    m_chattingRooms[number].SetRoomInTime(roomInTime);
    m_chattRoomLock.unlock();
    
    std::cout <<"["<< number <<"] [" << roomName << "] 생성" << std::endl;

    m_playersLock.lock();
    m_players[socket].StartLock();
    m_players[socket].SetState(ClientState::ROOM);
    m_players[socket].SetRoomNumber(number);
    m_players[socket].SetRoomInTime(roomInTime);
    m_players[socket].EndLock();
    m_playersLock.unlock();

    std::string enterMessage = "** ";
    enterMessage += m_players[socket].GetName();
    enterMessage += "님이 들어오셨습니다. (현재인원 1/";
    enterMessage += m_players[socket].GetChattingLog()[2]; 
    enterMessage += ")\n\r";

    m_playersLock.lock();
    m_players[socket].SendPacket(RenderMessageMacro::CREATEROOMSUCCESSMESSAGE, sizeof(RenderMessageMacro::CREATEROOMSUCCESSMESSAGE));
    m_players[socket].SendPacket(enterMessage.c_str(), static_cast<short>(enterMessage.size()));
    m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
    m_players[socket].ReceivePacket();
    m_playersLock.unlock();

    return true;
}

bool BaseServer::RequestRoomEnter(const SOCKET& socket)
{
    m_playersLock.lock();
    m_players[socket].StartLock();
    std::string message = { m_players[socket].GetChattingLog().cbegin(), m_players[socket].GetChattingLog().cend()};
    std::string userName = m_players[socket].GetName();
    m_players[socket].EndLock();
    m_playersLock.unlock();

    if (message[1] != ' ')
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return false;
    }

    std::string roomNumberMessage = { message.cbegin() + 2, message.cend() };

    int roomNumber =  atoi(roomNumberMessage.c_str());
    m_chattRoomLock.lock();
    auto iter = m_chattingRooms.find(roomNumber);
    auto enditer = m_chattingRooms.end();
    m_chattRoomLock.unlock();

    if (iter == enditer)
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::ROOMENTERFAILEDMESSAGE, sizeof(RenderMessageMacro::ROOMENTERFAILEDMESSAGE));
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return false;
    }

    m_chattRoomLock.lock();
    int total = m_chattingRooms[roomNumber].GetTotalPlayer();
    int max = m_chattingRooms[roomNumber].GetMaxUser();
    m_chattRoomLock.unlock();

    ///최대 인원 제한
    if (total == max)
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::ROOMENTERFULLMESSAGE, sizeof(RenderMessageMacro::ROOMENTERFULLMESSAGE));
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();

        return false;
    }

    m_chattRoomLock.lock();
    m_chattingRooms[roomNumber].PushAccessor(socket);
    std::vector<SOCKET> roomUserIndex = m_chattingRooms[roomNumber].GetAccessorIndex();
    m_chattingRooms[roomNumber].SetTotalPlayers(++total);
    m_chattRoomLock.unlock();

    ///입장 시간 기입
    std::string roomInTime = "";
    ///현재 시간
    char timebuffer[64];
    time_t currentTime = time(NULL);
    tm currentTm;
    localtime_s(&currentTm, &currentTime);

    strftime(timebuffer, sizeof(timebuffer), "%m-%d-%Y %X", &currentTm);
    roomInTime = timebuffer;

    m_playersLock.lock();
    m_players[socket].StartLock();
    m_players[socket].SetState(ClientState::ROOM);
    m_players[socket].SetRoomNumber(roomNumber);
    m_players[socket].SetRoomInTime(roomInTime);
    m_players[socket].EndLock();
    m_playersLock.unlock();
    
    char totalcount[8];
    char maxcount[8];
    _itoa_s(total, totalcount, 10);
    _itoa_s(max, maxcount, 10);

    std::string enterMessage = "\n\r** ";
    enterMessage +=  userName;
    enterMessage += "님이 입장하셨습니다.(";
    enterMessage += totalcount;
    enterMessage += " , ";
    enterMessage += maxcount;
    enterMessage += ")\n\r";    

    m_playersLock.lock();
    for (const auto& user : roomUserIndex)
    {
        m_players[user].SendPacket(enterMessage.c_str(), static_cast<unsigned short>( enterMessage.size()));
        m_players[user].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
    }
    m_players[socket].ReceivePacket();
    m_playersLock.unlock();

    return true;
}

bool BaseServer::RequestRoomList(const SOCKET& socket)
{
    m_playersLock.lock();
    m_players[socket].StartLock();
    std::string message = { m_players[socket].GetChattingLog().cbegin(), m_players[socket].GetChattingLog().cend() };
    m_players[socket].EndLock();
    m_playersLock.unlock();

    if (message != CommandMessage::ROOMLIST)
    {
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return false;
    }

    m_chattRoomLock.lock();
    size_t roomSize = m_chattingRooms.size();
    m_chattRoomLock.unlock();

    if (roomSize == 0)
    {
        /// 방이 없음.
        m_playersLock.lock();
        m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
        m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
        m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
        return true;
    }

    std::string roomsInfo = "";
    char roomNumber[8];
    char totalPlayer[8];
    char maxPlayer[8];
    m_chattRoomLock.lock();
    for (auto& room : m_chattingRooms)
    {
        _itoa_s(room.first, roomNumber, 10);
        room.second.StartLock();
        _itoa_s(room.second.GetTotalPlayer(), totalPlayer, 10);
        _itoa_s(room.second.GetMaxUser(), maxPlayer, 10);
        const char* name = room.second.GetName();
        room.second.EndLock();

        roomsInfo += '[';
        roomsInfo += roomNumber;
        roomsInfo += "] (";
        roomsInfo += totalPlayer;
        roomsInfo += ",";
        roomsInfo += maxPlayer;
        roomsInfo += ")";
        roomsInfo += name;
        roomsInfo += "\n\r";
    }
    m_chattRoomLock.unlock();

    m_playersLock.lock();
    m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
    m_players[socket].SendPacket(roomsInfo.c_str(), static_cast<unsigned short>(roomsInfo.size()));
    m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
    m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
    m_players[socket].ReceivePacket();
    m_playersLock.unlock();

    return true;
}

void BaseServer::LogOnCommandProcess()
{
    using namespace std::chrono_literals;

    while (true)
    {
        m_logOnLock.lock();
        if (m_logOn.empty() == true)
        {
            m_logOnLock.unlock();
            /// 로그인 하려는 유저가 있을 때 마다만 동작. 2000ms 한번 씩 로그인 유저 존재 검사
            std::this_thread::sleep_for(1500ms);
        }

        else
        {
            bool check = false;
            SOCKET socket = m_logOn.front();
            m_logOn.pop();
            m_logOnLock.unlock();
            m_playersLock.lock();
            m_players[socket].StartLock();
            /// 이름 추출
            const std::string name{ m_players[socket].GetChattingLog().cbegin() + sizeof(CommandMessage::LOGON), m_players[socket].GetChattingLog().cend() };
            m_players[socket].EndLock();
            /// 동일이름 유저 존재유무 확인

            for (const auto& player : m_players)
            {
                if (name == player.second.GetName())
                {
                    check = true;
                    break;
                }
            }
            m_playersLock.unlock();

           

            if (check == false)
            {
                m_playersLock.lock();
                m_players[socket].StartLock();
                m_players[socket].ClearChattingBuffer();
                m_players[socket].SetName(name.c_str());
                m_players[socket].SetState(ClientState::LOGON);
                m_players[socket].EndLock();
                m_playersLock.unlock();
                std::cout << name.c_str() << "[" << socket << "] LogOn" << std::endl;

                ///클라이언트에 로그인 성공 문구 출력
                m_playersLock.lock();
                m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
                m_players[socket].SendPacket(RenderMessageMacro::SUCCESSLOGONMESSAGE, sizeof(RenderMessageMacro::SUCCESSLOGONMESSAGE));
                m_players[socket].SendPacket(RenderMessageMacro::DIVIDELINEMESSAGE, sizeof(RenderMessageMacro::DIVIDELINEMESSAGE));
                m_players[socket].SendPacket(RenderMessageMacro::SELECTCOMMANDMESSAGE, sizeof(RenderMessageMacro::SELECTCOMMANDMESSAGE));
                m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
                m_players[socket].ReceivePacket();
                m_playersLock.unlock();
            }

            else
            {
                m_playersLock.lock();
                m_players[socket].StartLock();
                m_players[socket].ClearChattingBuffer();
                m_players[socket].EndLock();

                ///클라이언트에 로그인 실패 문구 출력
                m_players[socket].SendPacket(RenderMessageMacro::LOGONFAILED, sizeof(RenderMessageMacro::LOGONFAILED));
                m_players[socket].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
                m_players[socket].ReceivePacket();
                m_playersLock.unlock();
            }
        }
    }
}

bool BaseServer::Chatting(const SOCKET& socket)
{
    std::string userMessage = "[";
    m_playersLock.lock();
    m_players[socket].StartLock();
    int roomIndex = m_players[socket].GetRoomNumber();
    userMessage += m_players[socket].GetName();
    std::string message = { m_players[socket].GetChattingLog().cbegin(), m_players[socket].GetChattingLog().cend() };
    m_players[socket].EndLock();
    m_playersLock.unlock();

    userMessage += "]";
    userMessage += " ";
    userMessage += message;
    userMessage += "\n\r";

    m_chattRoomLock.lock();
    m_chattingRooms[roomIndex].StartLock();
    const std::vector<SOCKET> playersIndex = m_chattingRooms[roomIndex].GetAccessorIndex();
    m_chattingRooms[roomIndex].EndLock();
    m_chattRoomLock.unlock();

    for (int i = 0; i < playersIndex.size(); ++i)
    {
        if (playersIndex[i] == socket)
        {
            continue;
        }
        m_playersLock.lock();
        m_players[playersIndex[i]].SendPacket(userMessage.c_str(), static_cast<unsigned short>(userMessage.size()));
        m_players[playersIndex[i]].SendPacket(RenderMessageMacro::COMMANDWAITMESSAGE, sizeof(RenderMessageMacro::COMMANDWAITMESSAGE));
        m_playersLock.unlock();
    }


    return false;
}

bool BaseServer::ReassemblePacket(char* packet, const DWORD& bytes, const SOCKET& socket)
{
    if (packet == nullptr || bytes == 0)
        return false;

    bool flag = false;
    for (DWORD i = 0; i < bytes; ++i)
    {
        if (packet[i] == '\r\n' || packet[i] == '\n' || packet[i] == '\r')
        {
            StateWorkBranch(socket, m_players[socket].GetChattingLog());
            flag = true;
            break;
        }
        else
        {
            m_playersLock.lock();
            m_players[socket].StartLock();
            m_players[socket].PushChattingBuffer(packet[i]);
            m_players[socket].EndLock();
            m_playersLock.unlock();
        }
    }
    if (flag == false)
    {
        m_playersLock.lock();
        m_players[socket].ReceivePacket();
        m_playersLock.unlock();
    }
    return true;
}

bool BaseServer::Disconnect(SOCKET socket)
{
    m_playersLock.lock();
    std::string_view name = m_players[socket].GetName();
    m_players.erase(socket);
    m_playersLock.unlock();
    closesocket(socket);

    std::cout << "[" << socket << "] " << name << " 유저 접속 종료" << std::endl;
    return true;
}

void BaseServer::DisplayError(const char* msg)
{
    void* messageBuffer;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&messageBuffer, 0, NULL);
    std::cout << msg;
    std::wcout << L" 에러 " << messageBuffer << std::endl;
    LocalFree(messageBuffer);
}
