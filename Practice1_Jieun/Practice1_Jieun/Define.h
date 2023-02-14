#ifndef DEFINE_H
#define DEFINE_H

namespace InitailizeServer 
{
	constexpr int TOTALCORE = 8;
	constexpr int SERVERPORT = 9000;
	constexpr int MAX_BUFFERSIZE = 1024;
};

namespace InitailizePlayer
{
	constexpr int MAX_NAME = 16;
}

namespace RenderMessageMacro
{
	constexpr char AccessMessage[] = "** 안녕하세요. 텍스트 채팅 서버 Ver 0.1입니다.\n\r** 로그인 명령어(LOGIN)을 사용해주세요.\n\rLOGIN> ";
	constexpr char DivideLineMessage[] = "-------------------------------------------------------------------------\n\r";
	constexpr char SuccessLogOnMessage[] = "반갑습니다. 텍스트 채팅 서버 ver 0.1입니다.\n\r이용중 불편하신 점이 있으면 아래 이메일로 문의 바랍니다.\n\r감사합니다.\n\r\n\remail:jieun.kim23@nm-neo.com\n\r";
	constexpr char GuideMessage[] = "명령어 안내(H) 종료(X)\n\r";
	constexpr char CommandWaitMessage[] = ">>";
}

#endif // !DEFINE_H
 