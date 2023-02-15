

#ifndef CLIENT_H
#define CLIENT_H

enum class ClientState : char
{
	ACCESS, LOGON, ROOM, EXIT, END
};

class ClientInfo
{
private:
	SOCKET m_socket;
	WSAOVERLAPPED_EXTEND m_over;
	ClientState m_state;

	///Lock
	std::mutex m_clientLock;
	std::mutex m_receiveLock;

public:
	explicit ClientInfo();
	virtual ~ClientInfo() noexcept;

	void ReceivePacket();
	void SendPacket(const char* data, unsigned short packetSize);

	void StartLock();
	void EndLock();

	///Set
	void SetSocket(const SOCKET& s);
	void SetOverlappedExtend(const WSAOVERLAPPED_EXTEND& over);
	void SetOverlappedOperation(const EOperationType& operation);
	void SetState(const ClientState& state);

	///Get 
	const SOCKET GetSocket();
	const WSAOVERLAPPED_EXTEND& GetOverlappedExtend();
	const ClientState& GetState();
};

#endif // !CLIENT_H
