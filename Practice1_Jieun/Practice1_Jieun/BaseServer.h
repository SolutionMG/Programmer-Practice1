

#ifndef BASESERVER_H
#define BASESERVER_H


class PlayerInfo;


class BaseServer final
{
private:
	SOCKET m_listenSocket;
	HANDLE m_iocpHandle;

	/// 플레이어 관리용 변수
	std::unordered_map<SOCKET, PlayerInfo*> m_players;

public:
	explicit BaseServer();
	virtual ~BaseServer();


public:
	bool Initialize();
	bool Listen();
	bool OpenServer();


private:
	bool WorkProcess();


	bool Accept(WSAOVERLAPPED_EXTEND* over);
	bool AddNewClient(const SOCKET& socket);


	///텔넷 클라이언트로부터 받은 패킷 재조립
	bool ReassemblePacket(char* packet,const DWORD& bytes, const SOCKET& socket);
	bool ProcessPacket(const SOCKET& socket, char* word);
	bool Disconnect(SOCKET socket);


public:
	void DisplayError(const char* msg);
};


#endif // !BASESERV