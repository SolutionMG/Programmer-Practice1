#include "pch.h"
#include "BaseServer.h"
#include "PlayerInfo.h"
#include <algorithm>
#include <thread>

/// Static 변수
BaseServer::BaseServer()
{
    m_iocpHandle = NULL;
    m_listenSocket = INVALID_SOCKET;
    m_players.reserve(100);
}

BaseServer::~BaseServer()
{
    for_each(m_players.begin(), m_players.end(),
        [](std::pair<SOCKET, PlayerInfo*> player)
        {
            if (player.second != nullptr)
            {
                delete player.second;
                player.second = nullptr;
            }
        }
    );
}

bool BaseServer::Initialize()
{
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

    std::cout << "서버 초기화 완료" << std::endl;
    return true;
}

bool BaseServer::OpenServer()
{
    /// IOCP 객체 생성
    m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_listenSocket), m_iocpHandle, 0, 0);

    WSAOVERLAPPED_EXTEND over;
    Accept(&over);

    std::vector<std::thread> workerThreads;

    for (int i = 0; i < (TOTALCORE / 2); ++i)
    {
        workerThreads.emplace_back([&]() {WorkProcess(); });
    }

    ///타 기능 스레드 추가예정부분

    for (auto& wthread : workerThreads)
    {
        wthread.join();
    }

    closesocket(m_listenSocket);
    WSACleanup();

    return false;
}

bool BaseServer::WorkProcess()
{
    while (true)
    {
        DWORD bytes;
        ULONG_PTR completionKey;
        WSAOVERLAPPED* over;

        GetQueuedCompletionStatus(m_iocpHandle, &bytes, &completionKey, &over, INFINITE);
        SOCKET userKey = static_cast<SOCKET> (completionKey);
        WSAOVERLAPPED_EXTEND* overExtend = reinterpret_cast<WSAOVERLAPPED_EXTEND*>(over);

        switch (overExtend->opType)
        {
        case EOperationType::RECV:
        {
            if (bytes == 0) {
                Disconnect(userKey);
                break;
            }
            std::cout << "RECV Player " << m_players[userKey]->GetName() << userKey << std::endl;
            ReassemblePacket(overExtend->networkBuffer, bytes, userKey);
            m_players[userKey]->ReceivePacket();
        }
        break;

        case EOperationType::SEND:
        {
            if (bytes == 0) {
                Disconnect(userKey);
                break;
            }
        }
        break;

        case EOperationType::ACCEPT:
        {

            userKey = overExtend->socket;
            std::cout << "ACCEPT Player " << userKey << std::endl;
            AddNewClient(userKey);

            unsigned char test[24] = "Welcome!\r\n>\0";
            m_players[userKey]->SendPacket(test, sizeof(test));

            m_players[userKey]->ReceivePacket();
            Accept(overExtend);
        }
        break;
        }
    }
    return false;
}

bool BaseServer::ReassemblePacket(unsigned char* packet, const DWORD& bytes, const SOCKET& socket)
{
    DWORD remainData = bytes;
    
    while (remainData > 0)
    {
        if (remainData == 1)
        {
            if (packet[0] == '\n')
            {
                std::cout << m_players[socket]->GetChattingLog() << std::endl;
                break;
            }

            m_players[socket]->PushChattingBuffer(packet);
            packet += bytes;
            remainData -= bytes;
        }
    }
    
    //m_players[userKey]->SendPacket(static_cast<unsigned char*>(overExtend->networkBuffer), static_cast<unsigned short>(bytes));

    return false;
}

bool BaseServer::Listen()
{
    SOCKADDR_IN serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVERPORT);
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

    std::cout << "접속할 플레이어를 기다리고 있습니다." << std::endl;
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
//ReceivePacket 삭제

bool BaseServer::AddNewClient(const SOCKET& socket)
{
    m_players[socket] = new PlayerInfo;

    m_players[socket]->StartLock();
    m_players[socket]->SetSocket(socket);
    m_players[socket]->SetOverlappedOperation(EOperationType::RECV);
    m_players[socket]->EndLock();

    HANDLE returnValue2 = CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), m_iocpHandle, socket, 0);
    if (returnValue2 == NULL) {
        DisplayError("CreateIoCompletionPort AddNewClient()");
        return false;
    }
    return true;
}

bool BaseServer::Disconnect(SOCKET socket)
{
    socket;
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
