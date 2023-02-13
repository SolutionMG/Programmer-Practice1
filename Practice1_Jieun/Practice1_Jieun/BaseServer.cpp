#include "pch.h"
#include "BaseServer.h"
#include <thread>

BaseServer::BaseServer()
{
    m_listenSocket = INVALID_SOCKET;
    m_players.reserve(100);
}

BaseServer::~BaseServer()
{
}

bool BaseServer::Initialize()
{
    WSADATA wsaData;

    if ( WSAStartup(MAKEWORD(2, 2), &wsaData) != 0 )
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

    int socketOption = 1; //Nagle Off Option
    setsockopt(m_listenSocket, SOL_SOCKET, TCP_NODELAY, (const char*)(&socketOption), sizeof(socketOption));

    return true;
}

bool BaseServer::RunServer()
{
    Listen();

    

    return false;
}

bool BaseServer::WorkProcess()
{
    return true;
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

    int socketOption = 1; //Nagle Off Option
    setsockopt(m_listenSocket, SOL_SOCKET, TCP_NODELAY, (const char*)(&socketOption), sizeof(socketOption));

    ZeroMemory(&over->m_over, sizeof(over->m_over));

    
    over->m_opType = EOperationType::ACCEPT;
    over->m_socket = socket;


    bool returnValue = AcceptEx(m_listenSocket, socket, over->m_networkBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &over->m_over);
    if (returnValue == false)
    {
        DisplayError("AcceptEx Accept()");
        return false;
    }

    return true;
}

bool BaseServer::Receive(int playerIndex)
{
    playerIndex;
    return true;
}

bool BaseServer::Disconnect(int playerIndex)
{
    playerIndex;
    return true;
}

void BaseServer::DisplayError(const char* msg)
{
    WCHAR* messageBuffer;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&messageBuffer, 0, NULL);
    std::cout << msg;
    std::wcout << L" ¿¡·¯ " << messageBuffer << std::endl;
    LocalFree(messageBuffer);
}
