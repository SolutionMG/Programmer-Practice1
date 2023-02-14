

#ifndef ROOM_H
#define ROOM_H


class Room
{
private:
	/// ¹æ ¹øÈ£
	int m_index;

public:
	explicit Room();
	virtual ~Room() noexcept;
};

#endif // !ROOM_H
