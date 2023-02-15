

#ifndef BASESERVER_H
#define BASESERVER_H

#include <functional>

class PlayerInfo;
class ChattingRoom;

class BaseServer final
{
private:
	SOCKET m_listenSocket;
	HANDLE m_iocpHandle;

	///플레이어 관리용 변수
	std::mutex m_playersLock;
	std::unordered_map<SOCKET, PlayerInfo> m_players;

	///커맨드 별 함수 관리용 변수
	std::unordered_map<std::string_view, std::function<void(const SOCKET& socket)>> m_commandFunctions;

	///채팅방 관리용 변수
	std::mutex m_chattRoomLock;
	std::unordered_map<int, ChattingRoom> m_chattingRooms;
	int m_chatRoomindex;

	///로그온 프로세스
	std::mutex m_logOnLock;
	std::queue<SOCKET> m_logOn;

public:
	explicit BaseServer();
	virtual ~BaseServer() noexcept;

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

	///텔넷 클라이언트로부터 받은 패킷 재조립
	bool ReassemblePacket(char* packet, const DWORD& bytes, const SOCKET& socket);
	bool Disconnect(SOCKET socket);

	///플레이어 상태에 따른 패킷 명령 수행
	bool StateWorkBranch(const SOCKET& socket, const std::string_view& command);

	/// 플레이어의 명령어에 따른 패킷 명령 수행
	bool CommandWorkBranch(const SOCKET& socket, const std::string_view& request);

	/// 플레이어 커맨드 함수들...
	bool InitializeCommandFunction();
	bool ReqeustCommandList(const SOCKET& socket);
	bool RequestExit(const SOCKET& socket);
	bool RequestUserList(const SOCKET& socket);
	bool RequestRoomCreate(const SOCKET& socket);

	/// 로그온 진행 프로세스
	void LogOnCommandProcess();

	///채팅방에서의 대화
	//bool Chatting(const )

public:
	void DisplayError(const char* msg);
};


#endif // !BASESERV