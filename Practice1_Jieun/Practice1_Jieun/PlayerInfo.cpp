#include "pch.h"
#include "PlayerInfo.h"

PlayerInfo::PlayerInfo()
{
	strcpy_s(m_name, "");
}

PlayerInfo::~PlayerInfo()
{
}

const char* PlayerInfo::GetName()
{
	return m_name;
}

void PlayerInfo::SetName(const char* name)
{
	strcpy_s(m_name, name);
}
