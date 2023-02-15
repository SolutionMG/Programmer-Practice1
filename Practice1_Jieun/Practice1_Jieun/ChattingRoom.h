

#ifndef CHATTINGROOM_H
#define CHATTINGROOM_H
#include "Room.h"


class ChattingRoom: public Room
{
private:
	/// 채팅방 총인원 수 
	int m_totalPlayers;
	/// 해당 채팅방 접속자들의 고유 인덱스
	std::vector<int> m_accessorIndex;

public:
	explicit ChattingRoom();
	virtual ~ChattingRoom() noexcept;

public:
	
	void PushAccessor(const int& index);
	void PopAccessor(const int index);

	///Set
	void SetTotalPlayers(const int& totalPlayer);

	///Get
	const int& GetTotalPlayer();
	const std::vector<int>& GetAccessorIndex();
};

#endif // !CHATTINGROOM_H
