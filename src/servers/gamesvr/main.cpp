#include <stdio.h>
#include "../../common/log/Logger.h"
#include "../../common/stdafx.h"

int main()
{
	Logger& log = Logger::Instance();
	log.Init("./log", 0);

	int loop = 10000;

	while (loop > 0)
	{
		LOG_INFO("hello gameserver:%d", loop);
		--loop;
	}

	printf("log end!\n");

	while (true)
	{
		Sleep(3000);
	}

	return 0;
}