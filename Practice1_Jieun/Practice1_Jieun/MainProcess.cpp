#include"pch.h"
#include "BaseServer.h"


int main()
{
	BaseServer baseServer;

	if (baseServer.Initialize() == false) {
		std::cout << "baseServer.Initialize() 실패" << std::endl;
		return -1;
	}

	if (baseServer.RunServer() == false)
	{
		std::cout << "baseServer.RunServer() 실패" << std::endl;
		return -1;
	}

	return 0;
}