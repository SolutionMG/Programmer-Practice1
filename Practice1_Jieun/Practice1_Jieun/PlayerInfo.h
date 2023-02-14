

#ifndef PLAYERINFO_H
#define PLAYERINFO_H
#include "ClientInfo.h"


class PlayerInfo final : public ClientInfo
{
private:
	///플레이어 고유 이름
	char m_name[InitailizePlayer::MAX_NAME];
	///플레이어가 송신 전 입력중인 문자열
	std::vector<char> m_chattingBuffer;

public:
	explicit PlayerInfo();
	virtual ~PlayerInfo() noexcept;


	///Set
	void SetName(const char* name);


	///Get
	const char* GetName();
	const std::string_view GetChattingLog();


	void PushChattingBuffer(char word);
	void ClearChattingBuffer();

};

#endif // !PLAYERINFO_H
