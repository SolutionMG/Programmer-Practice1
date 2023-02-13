#ifndef PLAYERINFO_H
#define PLAYERINFO_H

constexpr int MAX_NAME = 16;

class PlayerInfo
{
public:
	explicit PlayerInfo();
	virtual ~PlayerInfo();

private:
	///플레이어 고유 번호
	unsigned int m_Index;	
	///플레이어 고유 이름
	char m_name[MAX_NAME];
};

#endif // !PLAYERINFO_H
