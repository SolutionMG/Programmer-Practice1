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

void PlayerInfo::PushChattingBuffer(unsigned char word[])
{
	m_chattingBuffer.emplace_back(word);
}

void PlayerInfo::ClearChattingBuffer()
{
	m_chattingBuffer.clear();
}

bool PlayerInfo::CheckChattingEnd()
{
	if (m_chattingBuffer.empty() == true)
		return false;

	if ((m_chattingBuffer[m_chattingBuffer.size() - 1][0]) == '\n')
	{
		std::cout << "Send" << std::endl;
		return true;

	}
	return false;
}

const std::string PlayerInfo::GetChattingLog()
{
	std::string str = "";
	for (auto& w : m_chattingBuffer)
		str += static_cast<std::string>(reinterpret_cast<char*>(w));

	return str;
}
