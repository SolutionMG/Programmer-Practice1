

#ifndef CLIENT_H
#define CLIENT_H

class ClientInfo
{
private:
	SOCKET m_socket;
	WSAOVERLAPPED_EXTEND m_over;
	
	///Lock
	std::mutex m_clientLock;

public:
	explicit ClientInfo();

	void ReceivePacket();
	void SendPacket(const char* data, unsigned short packetSize);

	void StartLock();
	void EndLock();

	///Set
	void SetSocket(const SOCKET& s);
	void SetOverlappedExtend(const WSAOVERLAPPED_EXTEND& over);
	void SetOverlappedOperation(const EOperationType& operation);

	///Get 
	const SOCKET GetSocket();
	const WSAOVERLAPPED_EXTEND& GetOverlappedExtend();
};

#endif // !CLIENT_H
