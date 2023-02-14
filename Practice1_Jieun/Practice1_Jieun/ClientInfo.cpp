#include "pch.h"
#include "ClientInfo.h"

ClientInfo::ClientInfo() : m_socket{INVALID_SOCKET}, m_over{ WSAOVERLAPPED_EXTEND() }
{
	ZeroMemory(&m_over, sizeof(m_over));
	m_over.wsaBuffer.buf = m_over.networkBuffer;
	m_over.wsaBuffer.len = InitailizeServer::MAX_BUFFERSIZE ;
}

void ClientInfo::ReceivePacket()
{
	/// Overlapped Receive 요청
	ZeroMemory(&m_over, sizeof(m_over));
	m_over.wsaBuffer.buf = m_over.networkBuffer;
	m_over.wsaBuffer.len = InitailizeServer::MAX_BUFFERSIZE;
	m_over.opType = EOperationType::RECV;
	DWORD flag = 0;
	WSARecv(m_socket, &m_over.wsaBuffer, 1, NULL, &flag, &m_over.over, NULL);
}

void ClientInfo::SendPacket(char* data, unsigned short packetSize)
{
	/// Overlapped Send 요청
	WSAOVERLAPPED_EXTEND* over = new WSAOVERLAPPED_EXTEND;
	over->opType = EOperationType::SEND;
	ZeroMemory(&over->over, sizeof(over->over));
	memcpy_s(over->networkBuffer, sizeof(over->networkBuffer), data, packetSize);
	over->wsaBuffer.buf = over->networkBuffer;
	over->wsaBuffer.len = packetSize;
	WSASend(m_socket, &over->wsaBuffer, 1, 0, 0, &over->over, 0);

}

void ClientInfo::StartLock()
{
	m_clientLock.lock();
}

void ClientInfo::EndLock()
{
	m_clientLock.unlock();
}

const SOCKET ClientInfo::GetSocket()
{
	return m_socket;
}

void ClientInfo::SetSocket(const SOCKET& s)
{
	m_socket = s;
}

const WSAOVERLAPPED_EXTEND& ClientInfo::GetOverlappedExtend()
{
	return m_over;
}

void ClientInfo::SetOverlappedExtend(const WSAOVERLAPPED_EXTEND& over)
{
	memcpy_s(&m_over, sizeof(m_over), &over, sizeof(over));
}
void ClientInfo::SetOverlappedOperation(const EOperationType& operation)
{
	m_over.opType = operation;
}

