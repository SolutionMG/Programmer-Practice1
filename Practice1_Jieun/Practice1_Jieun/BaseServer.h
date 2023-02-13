#ifndef BASESERVER_H
#define BASESERVER_H

class PlayerInfo;

class BaseServer final
{
private:
	static SOCKET m_listenSocket;
	static HANDLE ms_iocpHandle;

	/// 플레이어 관리용 변수
	static std::unordered_map<SOCKET, PlayerInfo*> m_players;

public:
	explicit BaseServer();
	virtual ~BaseServer();

public:
	bool Initialize();
	bool Listen();
	bool OpenServer();

private:
	bool Disconnect(SOCKET socket);

	static bool WorkProcess();
	static bool PacketReassembly(unsigned char* packet, DWORD bytes, SOCKET socket);

	static bool Accept(WSAOVERLAPPED_EXTEND* over);
	static bool ReceivePacket(SOCKET socket);
	static bool SendPacket(SOCKET socket, void* data, unsigned short packetSize);

	static bool AddNewClient(SOCKET socket);


public:
	static void DisplayError(const char* msg);
};


#endif // !BASESERV