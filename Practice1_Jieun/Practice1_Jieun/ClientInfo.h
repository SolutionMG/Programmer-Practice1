#ifndef CLIENT_H
#define CLIENT_H

class ClientInfo
{
private:
	SOCKET m_socket;
	WSAOVERLAPPED_EXTEND m_over;
	std::mutex m_clientLock;
	
public:
	explicit ClientInfo();
	virtual ~ClientInfo();

	void StartLock();
	void EndLock();

	///Get
	const SOCKET GetSocket();
	WSAOVERLAPPED_EXTEND& GetOverlappedExtend();

	///Set
	void SetSocket(const SOCKET& s);
	void SetOverlappedExtend(const WSAOVERLAPPED_EXTEND& over);


};

#endif // !CLIENT_H
