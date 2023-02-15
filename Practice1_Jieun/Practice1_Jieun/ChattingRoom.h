

#ifndef CHATTINGROOM_H
#define CHATTINGROOM_H
#include "Room.h"


class ChattingRoom: public Room
{
private:
	/// 채팅방 총인원 수 
	int m_totalPlayers;
	/// 해당 채팅방 접속자들의 소켓들
	std::vector<SOCKET> m_accessorIndex;

public:
	explicit ChattingRoom();
	virtual ~ChattingRoom() noexcept;

public:
	
	void PushAccessor(const SOCKET& socket);
	void PopAccessor(const SOCKET& socket);

	///Set
	void SetTotalPlayers(const int& totalPlayer);

	///Get
	const int& GetTotalPlayer() const;
	const std::vector<SOCKET>& GetAccessorIndex();
};

#endif // !CHATTINGROOM_H
