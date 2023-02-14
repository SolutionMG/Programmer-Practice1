

#ifndef CHATTINGROOM_H
#define CHATTINGROOM_H
#include "Room.h"


class ChattingRoom: public Room
{
private:
	/// 채팅방 총인원 수 
	int totalPlayers;
	/// 해당 채팅방 접속자들의 고유 인덱스
	std::vector<int> accessorIndex;

public:
	explicit ChattingRoom();
};

#endif // !CHATTINGROOM_H
