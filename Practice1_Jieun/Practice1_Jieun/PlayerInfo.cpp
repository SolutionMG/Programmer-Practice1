#include "pch.h"
#include "PlayerInfo.h"

PlayerInfo::PlayerInfo()
{
	m_Index = 0;
	strcpy_s(m_name, "");
}

PlayerInfo::~PlayerInfo()
{
}
