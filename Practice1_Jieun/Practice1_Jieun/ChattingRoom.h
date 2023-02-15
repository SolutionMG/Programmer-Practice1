

#ifndef CHATTINGROOM_H
#define CHATTINGROOM_H
#include "Room.h"


class ChattingRoom: public Room
{
private:
	/// 해당 채팅방 접속자들의 소켓들
	std::vector<SOCKET> m_accessorIndex;

public:
	explicit ChattingRoom();
	virtual ~ChattingRoom() noexcept;

public:
	
	void PushAccessor(const SOCKET& socket);
	void PopAccessor(const SOCKET& socket);

	///Set

	///Get
	const std::vector<SOCKET>& GetAccessorIndex() const;
};

#endif // !CHATTINGROOM_H
