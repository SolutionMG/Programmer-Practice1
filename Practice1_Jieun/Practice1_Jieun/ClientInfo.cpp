#include "pch.h"
#include "ClientInfo.h"

ClientInfo::ClientInfo()
{
	m_socket = INVALID_SOCKET;
	m_over = WSAOVERLAPPED_EXTEND();
	ZeroMemory(&m_over, sizeof(m_over));
}

ClientInfo::~ClientInfo()
{
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

WSAOVERLAPPED_EXTEND& ClientInfo::GetOverlappedExtend()
{
	return m_over;
}


void ClientInfo::SetOverlappedExtend(const WSAOVERLAPPED_EXTEND& over)
{
	memcpy_s(&m_over, sizeof(m_over), &over, sizeof(over));
}
