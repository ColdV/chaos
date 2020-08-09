#include <stdio.h>
#include "../../common/log/Logger.h"
#include "../../common/stdafx.h"
#include "../../common/net/Event.h"
#include<functional>
#include "../../common/common.h"

class Test
{
public:
	void ListenCb(chaos::NetEvent* ev, chaos::NetEvent*, void* arg);
};


void Test::ListenCb(chaos::NetEvent* ev, chaos::NetEvent* pNewEv, void* arg)
{
	printf("socket:%d, newsocket:%d\n", ev->GetSocket().GetFd(), pNewEv->GetSocket().GetFd());
}

#define IP "192.168.0.101"
#define PORT 3307

int main()
{
	Logger& log = Logger::Instance();
	log.Init("./log", 0);

	//int loop = 10000;

	//while (loop > 0)
	//{
	//	LOG_INFO("hello gameserver:%d", loop);
	//	--loop;
	//}

	//printf("log end!\n");

	//while (true)
	//{
	//	Sleep(3000);
	//}

	//std::function<void(int a, int b)> a;
	//if (!a)
	//	printf("yes!\n");

	//a(1,2);

	chaos::EventCentre* p = new chaos::EventCentre;

	p->Init();

	chaos::Socket s(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	chaos::Listener* ev = new chaos::Listener(p, s.GetFd());

	printf("listen socket:%d\n", s.GetFd());

	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &sa.sin_addr);

	if (0 != ev->Listen((sockaddr*)&sa, sizeof(sa)))
	{
		printf("listen failed! err:%d\n", WSAGetLastError());
		return -1;
	}

	Test t;

	auto cb = std::bind(&Test::ListenCb, &t, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	ev->SetListenerCb(cb, NULL);
	p->RegisterEvent(ev);

	p->EventLoop();

	return 0;
}