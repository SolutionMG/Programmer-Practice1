

#ifndef ROOM_H
#define ROOM_H


class Room
{
private:
	std::mutex m_roomLock;

	/// 방 번호
	int m_index;
	/// 방 이름
	char m_name[InitializeRoom::MAX_ROOMNAME];
	/// 최대 인원
	int m_maxUser;

public:
	explicit Room();
	virtual ~Room() noexcept;

	void StartLock();
	void EndLock();

	///Set
	void SetName(const char* name);
	void SetIndex(const int& index);
	void SetMaxUser(const int& num);

	///Get
	const char* GetName() const;
	const int& GetIndex();
	const int& GetMaxUser();
};

#endif // !ROOM_H
