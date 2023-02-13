#ifndef CHATTINGROOM_H
#define CHATTINGROOM_H

class ChattingRoom
{
public:
	explicit ChattingRoom();
	virtual ~ChattingRoom();

private:
	/// 채팅방 총인원 수 
	int totalPlayers;
	/// 해당 채팅방 접속자들의 고유 인덱스
	std::vector<int> accessorIndex; 
};

#endif // !CHATTINGROOM_H
