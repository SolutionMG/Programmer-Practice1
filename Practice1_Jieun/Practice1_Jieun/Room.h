

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
	/// 채팅방 총인원 수 
	int m_totalPlayers;

public:
	explicit Room();
	virtual ~Room() noexcept;

	void StartLock();
	void EndLock();

	///Set
	void SetName(const char* name);
	void SetIndex(const int& index);
	void SetMaxUser(const int& num);
	void SetTotalPlayers(const int& totalPlayer);

	///Get
	const char* GetName() const;
	const int& GetIndex();
	const int& GetMaxUser() const;
	const int& GetTotalPlayer() const;

};

#endif // !ROOM_H
