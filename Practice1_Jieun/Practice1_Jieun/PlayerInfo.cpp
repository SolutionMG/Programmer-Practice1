#include "pch.h"
#include "PlayerInfo.h"

PlayerInfo::PlayerInfo()
{
	strcpy_s(m_name, "Default");
	m_chattingBuffer.reserve(MAX_BUFFERSIZE);
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
	name;
	strcpy_s(m_name, name);
}

void PlayerInfo::PushChattingBuffer(char word)
{
	m_chattingBuffer.emplace_back(word);
}

void PlayerInfo::ClearChattingBuffer()
{
	m_chattingBuffer.clear();
}

const std::string PlayerInfo::GetChattingLog()
{
	std::string str = {m_chattingBuffer.begin(), m_chattingBuffer.end()};
	std::cout << str;
	return str;
}
