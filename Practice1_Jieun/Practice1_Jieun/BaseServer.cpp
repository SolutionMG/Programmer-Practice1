#include "pch.h"
#include "BaseServer.h"
#include "PlayerInfo.h"
#include <algorithm>
#include <thread>

/// Static 변수
HANDLE BaseServer::ms_iocpHandle = NULL;
std::unordered_map<SOCKET, PlayerInfo*> BaseServer::m_players;
SOCKET BaseServer::m_listenSocket = INVALID_SOCKET; ;

BaseServer::BaseServer()
{
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
    WSADATA wsaData;
    int returnValue = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (returnValue != 0 )
    {
        DisplayError("WSAStartup Initialize()");
        return false;
    }

    m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    if ( m_listenSocket == INVALID_SOCKET )
    {
        DisplayError( "WSASocket Initialize()" );
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
    return true;
}

bool BaseServer::OpenServer()
{
    /// IOCP 객체 생성
    ms_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0); 
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_listenSocket), ms_iocpHandle, 0, 0);
    
    WSAOVERLAPPED_EXTEND over;
    Accept(&over);

    std::vector<std::thread> workerThreads;

    for (int i = 0; i < (TOTALCORE / 2); ++i)
    {
        workerThreads.emplace_back(WorkProcess);
    }
    
    ///타 기능 스레드 추가예정부분

    for (auto& wthread : workerThreads)
    {
        wthread.join();
    }

    return false;
}

bool BaseServer::WorkProcess()
{
    while (true)
    {
        DWORD bytes;
        ULONG_PTR completionKey;
        WSAOVERLAPPED* over;

        GetQueuedCompletionStatus(ms_iocpHandle, &bytes, &completionKey, &over, INFINITE);
        SOCKET user_key = static_cast<SOCKET> ( completionKey );
        WSAOVERLAPPED_EXTEND* overExtend = reinterpret_cast<WSAOVERLAPPED_EXTEND*>( over );

        switch (overExtend->m_opType)
        {
            case EOperationType::RECV:
            {
                std::cout << "RECV Player " << user_key << std::endl;
                PacketReassembly(overExtend->m_networkBuffer, bytes, user_key);
                ReceivePacket(user_key);
            }
            break;

            case EOperationType::SEND:
            {

            }
            break;
            
            case EOperationType::ACCEPT:
            {
                user_key = static_cast<SOCKET>(overExtend->m_wsaBuffer.len);
                std::cout << "ACCEPT Player "<< user_key << std::endl;

                AddNewClient(user_key);
                ReceivePacket(user_key);
                Accept(overExtend);
            }
            break;
        }
    }
    return false;
}

bool BaseServer::PacketReassembly(unsigned char* packet, DWORD bytes, SOCKET socket)
{
    packet;
    bytes;
    socket;
    
    //while(remainData > 0)
    //{
    //   int packet_size = 
    //}
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

    if ( returnValue != 0 )
    {
        DisplayError("bind Listen()");
        return false;
    }

    returnValue = listen(m_listenSocket, SOMAXCONN);
    if ( returnValue != 0 )
    {
        DisplayError("listen Listen()");
        return false;
    }

    return true;
}

bool BaseServer::Accept(WSAOVERLAPPED_EXTEND* over)
{
    SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if ( socket == INVALID_SOCKET )
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

    ZeroMemory(&over->m_over, sizeof(over->m_over));

    DWORD bytes;
    over->m_opType = EOperationType::ACCEPT;
    over->m_socket = socket;
    bool returnValue2 = AcceptEx(m_listenSocket, socket, over->m_networkBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, &over->m_over);
    if ( returnValue2 == false ) 
    {
        ///작업이 성공적으로 시작, 아직 진행 중
        if (WSAGetLastError() == ERROR_IO_PENDING)
            return true;

        DisplayError("AcceptEx Accept()");
        return false;
    }

    return true;
}

bool BaseServer::ReceivePacket(SOCKET socket)
{
    ///Recv 버퍼 초기화 후 Recv 비동기 요청
    ZeroMemory(&m_players[socket]->GetOverlappedExtend().m_over, sizeof(m_players[socket]->GetOverlappedExtend().m_over));
    m_players[socket]->GetOverlappedExtend().m_opType = EOperationType::RECV;
    DWORD flag = 0;
    WSARecv(socket, &m_players[socket]->GetOverlappedExtend().m_wsaBuffer, 1, NULL, &flag, &m_players[socket]->GetOverlappedExtend().m_over, NULL);

    return true;
}

bool BaseServer::SendPacket(SOCKET socket, void* data, unsigned short packetSize)
{
    ///수신할 데이터 networkbuffer에 복사 후 Send 비동기 요청
    WSAOVERLAPPED_EXTEND* over = new WSAOVERLAPPED_EXTEND;
    over->m_opType = EOperationType::SEND;
    ZeroMemory(&over->m_over, sizeof(over->m_over));
    memcpy_s(over->m_networkBuffer, sizeof(over->m_networkBuffer), data, packetSize);
    over->m_wsaBuffer.buf = reinterpret_cast<char*>(over->m_networkBuffer);
    over->m_wsaBuffer.len = packetSize;
    WSASend(socket, &over->m_wsaBuffer, 1, 0, 0, &over->m_over, 0);

    return true;
}

bool BaseServer::AddNewClient(SOCKET socket)
{
    ///Nagle Off Option
    m_players[socket] = new PlayerInfo;

    m_players[socket]->StartLock();
    m_players[socket]->SetSocket(socket);
    m_players[socket]->GetOverlappedExtend().m_opType = EOperationType::RECV;
    std::cout << (int)m_players[socket]->GetOverlappedExtend().m_opType << std::endl;
    m_players[socket]->EndLock();

    HANDLE returnValue2 = CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), ms_iocpHandle, socket, 0);
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
