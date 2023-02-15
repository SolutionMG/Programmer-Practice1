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
	constexpr unsigned short MAX_ROOMNAME = 32;
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

	constexpr char GUIDEMESSAGE[] = "[H] - 명령어 목록 안내\n\r[X] - 접속종료\n\r[US] - 접속 유저 목록\n\r[O] [최대인원] [방제목] - 방생성\n\r";

	constexpr char CREATEROOMFAILEDMESSAGE[] = "** 이미 동일한 이름의 방이 존재합니다. 다른 이름을 사용해주세요.\n\r";
	constexpr char CREATEROOMSUCCESSMESSAGE[] = "** 대화방이 개설되었습니다.\n\r";
}

namespace CommandMessage
{
	constexpr char LOGON[] = "LOGIN";			//로그인 명령어		O
	constexpr char COMMANDLIST[] = "H";			//명령어 안내		O
	constexpr char USERLIST[] = "US";			//이용자 목록 보기	O
	constexpr char ROOMLIST[] = "LT";			//대화방 목록 보기
	constexpr char ROOMINFO[] = "ST";			//방 정보 보기
	constexpr char PLAYERINFO[] = "P";			//이용자 정보 보기
	constexpr char SECRETMESSAGE[] = "TO";		//귓속말
	constexpr char ROOMCREATE[] = "O";			//방생성
	constexpr char ROOMENTER[] = "J";			//방입장
	constexpr char EXIT[] = "X";				//종료
	constexpr char ROOMOUT[] = "ROOMOUT";		//방나가기

}

#endif // !DEFINE_H
