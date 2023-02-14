

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

	///로그온 프로세스
	std::mutex m_logOnLock;
	std::queue<SOCKET> m_logOn; 

public:
	explicit BaseServer();
	virtual ~BaseServer();


public:
	bool Initialize();
	bool Listen();
	bool OpenServer();


private:
	/// 메인 IOCP 프로세스 (Accept, Recv, Send)
	bool MainWorkProcess();

	/// 플레이어 접속 처리 함수
	bool Accept(WSAOVERLAPPED_EXTEND* over);
	bool AddNewClient(const SOCKET& socket);

	///패킷에 따른 명령 수행
	bool CommandWorkBranch(const SOCKET& socket, const std::string_view& command);

	/// 로그온 진행 프로세스
	bool LogOnCommandProcess();

	///텔넷 클라이언트로부터 받은 패킷 재조립
	bool ReassemblePacket(char* packet,const DWORD& bytes, const SOCKET& socket);
	bool Disconnect(SOCKET socket);


public:
	void DisplayError(const char* msg);
};


#endif // !BASESERV