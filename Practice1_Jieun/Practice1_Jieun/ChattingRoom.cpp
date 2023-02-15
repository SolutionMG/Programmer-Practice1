#include "pch.h"
#include "ChattingRoom.h"
#include <algorithm>

ChattingRoom::ChattingRoom() : m_totalPlayers(0)
{
	m_accessorIndex.reserve(20);
}
ChattingRoom::~ChattingRoom() noexcept = default;

void ChattingRoom::PushAccessor( const SOCKET& socket)
{
	m_accessorIndex.push_back(socket);
	std::cout << socket << "Ãß°¡" << std::endl;
}

void ChattingRoom::PopAccessor(const SOCKET& socket)
{
	auto iter = find(m_accessorIndex.begin(), m_accessorIndex.end(), socket);
	if (iter != m_accessorIndex.end())
		m_accessorIndex.erase(iter);
}

void ChattingRoom::SetTotalPlayers(const int& totalPlayer)
{
	m_totalPlayers = totalPlayer;
}

const int& ChattingRoom::GetTotalPlayer() const
{
	return m_totalPlayers;
}

const std::vector< SOCKET >& ChattingRoom::GetAccessorIndex()
{
	return m_accessorIndex;
}
