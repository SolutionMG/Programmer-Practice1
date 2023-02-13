#ifndef BASESERVER_H
#define BASESERVER_H

constexpr int MAX_BUFFERSIZE = 1024;
constexpr int SERVERPORT = 9000;

class PlayerInfo;

///OPERATION TYPE
enum class EOperationType : char
{
	RECV, SEND, ACCEPT, END
};

///EXTEND OVERLAPPED
struct WSAOVERLAPPED_EXTEND
{
	WSAOVERLAPPED	m_over;
	WSABUF			m_wsaBuffer[1];
	unsigned char	m_networkBuffer[MAX_BUFFERSIZE];
	EOperationType	m_opType;
	SOCKET			m_socket;
};

class BaseServer
{
public:
	explicit BaseServer();
	virtual ~BaseServer();

public:
	bool Initialize();
	bool RunServer();

	bool WorkProcess();

public:
	bool Listen();
	bool Accept(WSAOVERLAPPED_EXTEND* over);
	bool Receive(int playerIndex);
	bool Disconnect(int playerIndex);

public:
	void DisplayError(const char* msg);

private:
	SOCKET m_listenSocket;
	std::unordered_map<unsigned int, PlayerInfo*> m_players;
};


#endif // !BASESERV