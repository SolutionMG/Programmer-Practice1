#ifndef DEFINE_H
#define DEFINE_H

namespace InitailizeServer
{
	constexpr unsigned short TOTALCORE = 8;
	constexpr unsigned short SERVERPORT = 9000;
	constexpr unsigned short MAX_BUFFERSIZE = 1024;
};

namespace InitailizePlayer
{
	constexpr unsigned short MAX_NAME = 32;
}

namespace InitializeRoom
{
	constexpr unsigned short MAX_ROOMPLAYER = 20;
	constexpr unsigned short MIN_ROOMPLAYER = 2;
}

namespace RenderMessageMacro
{
	constexpr char ACCESSMESSAGE[] = "** 안녕하세요. 텍스트 채팅 서버 Ver 0.1입니다.\n\r";
	constexpr char LOGONREQUEST[] = "** 로그인 명령어(LOGIN)을 사용해주세요.\n\r";
	constexpr char LOGONFAILED[] = "** 아이디를 이미 사용중입니다. 다른 아이디를 사용해주세요.\n\r";
	constexpr char DIVIDELINEMESSAGE[] = "-------------------------------------------------------------------------\n\r";
	constexpr char SUCCESSLOGONMESSAGE[] = "반갑습니다. 텍스트 채팅 서버 ver 0.1입니다.\n\r이용중 불편하신 점이 있으면 아래 이메일로 문의 바랍니다.\n\r감사합니다.\n\r\n\remail:jieun.kim23@nm-neo.com\n\r";
	constexpr char SELECTCOMMANDMESSAGE[] = "명령어 안내(H) 종료(X)\n\r";
	constexpr char COMMANDWAITMESSAGE[] = ">>";

	constexpr char GUIDEMESSAGE[] = "[H] - 명령어 목록 안내\n\r[X] - 접속종료\n\r[US] - 접속 유저 목록\n\r";
}

namespace CommandMessage
{
	constexpr char LOGON[] = "LOGIN";
	constexpr char COMMANDLIST[] = "H";
	constexpr char USERLIST[] = "US";
	constexpr char ROOMLIST[] = "LT";
	constexpr char ROOMINFO[] = "ST";
	constexpr char PLAYERINFO[] = "PF";
	constexpr char SECRETMESSAGE[] = "TO";
	constexpr char ROOMCREATE[] = "O";
	constexpr char ROOMENTER[] = "J";
	constexpr char EXIT[] = "X";
	constexpr char ROOMOUT[] = "ROOMOUT";

}

#endif // !DEFINE_H
