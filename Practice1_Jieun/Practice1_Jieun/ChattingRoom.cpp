#include "pch.h"
#include "ChattingRoom.h"
#include <algorithm>

ChattingRoom::ChattingRoom() : m_totalPlayers(0)
{
	m_accessorIndex.reserve(20);
}
ChattingRoom::~ChattingRoom() noexcept = default;

void ChattingRoom::PushAccessor( const int& index )
{
	m_accessorIndex.push_back(index);
}

void ChattingRoom::PopAccessor(const int index)
{
	auto iter = find(m_accessorIndex.begin(), m_accessorIndex.end(), index);
	if (iter != m_accessorIndex.end())
		m_accessorIndex.erase(iter);
}

void ChattingRoom::SetTotalPlayers(const int& totalPlayer)
{
	m_totalPlayers = totalPlayer;
}

const int& ChattingRoom::GetTotalPlayer()
{
	return m_totalPlayers;
}

const std::vector< int >& ChattingRoom::GetAccessorIndex()
{
	return m_accessorIndex;
}
