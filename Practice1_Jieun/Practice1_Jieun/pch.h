// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H

///network
#include<WS2tcpip.h>
#include<MSWSock.h>
#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib, "mswsock.lib")

///C++
#include<iostream>
#include<unordered_map>
#include<vector>
#include<queue>
#include<mutex>

///USER
#include "Define.h"

///OPERATION TYPE
enum class EOperationType : char
{
	RECV, SEND, ACCEPT, END
};

///EXTEND OVERLAPPED
struct WSAOVERLAPPED_EXTEND
{
	WSAOVERLAPPED	m_over;
	WSABUF			m_wsaBuffer;
	unsigned char	m_networkBuffer[MAX_BUFFERSIZE];
	EOperationType	m_opType;
	SOCKET			m_socket;
};

#endif //PCH_H
