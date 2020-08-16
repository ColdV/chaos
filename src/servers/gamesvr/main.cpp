#include <stdio.h>
#include "../../common/log/Logger.h"
#include "../../common/stdafx.h"
#include "../../common/net/Event.h"
#include <functional>
#include "../../common/common.h"
#include "../../common/net/Timer.h"

class Test
{
public:
	void ListenCb(chaos::Listener* ev, chaos::Connecter*, void* arg);

	void ReadCb(chaos::Connecter* ev, int nTransBytes, void* arg);

	void WriteCb(chaos::Connecter* ev, int nTransBytes, void* arg);

	void TimerCb(chaos::Event* pev, short ev, void* arg);
};


void Test::ListenCb(chaos::Listener* ev, chaos::Connecter* pConner, void* arg)
{
	if (pConner)
	{
		pConner->SetCallback(std::bind(&Test::ReadCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL,
			std::bind(&Test::WriteCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);
	}

	printf("socket:%d, newsocket:%d\n", ev->GetSocket().GetFd(), pConner->GetSocket().GetFd());
}


void Test::ReadCb(chaos::Connecter* ev, int nTransBytes, void* arg)
{
	printf("read socket:%d trans bytes:%d\n", ev->GetSocket().GetFd(), nTransBytes);

	int readable = ev->GetReadableSize();
	char* buf = new char[readable];
	if (!buf)
		return;

	ev->ReadBuffer(buf, readable);

	printf("read socket recv data:%s\n", buf);
	delete[] buf;
}


void Test::WriteCb(chaos::Connecter* ev, int nTransBytes, void* arg)
{
	printf("write socket:%d trans bytes:%d\n", ev->GetSocket().GetFd(), nTransBytes);
}


void Test::TimerCb(chaos::Event* pev, short ev, void* arg)
{
	printf("timer:%d call back\n", pev->GetEvKey().timerId);
}


#define IP "192.168.0.101"
#define PORT 3307


int main()
{
	//---------log test begin-----------//

	//Logger& log = Logger::Instance();
	//log.Init("./log", 0);

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

	//---------log test end-----------//


	//---------net test begin-----------//
	chaos::EventCentre* p = new chaos::EventCentre;

	p->Init();

	chaos::Socket* s = new chaos::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	chaos::Listener* ev = new chaos::Listener(p, s->GetFd());

	printf("listen socket:%d\n", s->GetFd());

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PORT);
	//inet_pton(AF_INET, IP, &sa.sin_addr);

	if (0 != ev->Listen((sockaddr*)&sa, sizeof(sa)))
	{
		printf("listen failed! err:%d\n", WSAGetLastError());
		return -1;
	}

	Test t;

	auto cb = std::bind(&Test::ListenCb, &t, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	ev->SetListenerCb(cb, NULL);
	p->RegisterEvent(ev);

	timer_id timerId = chaos::Timer::CreateTimerID();
	chaos::TimerEvent* timerEv = new chaos::TimerEvent(p, timerId, 3, true);
	timerEv->SetEventCallback(std::bind(&Test::TimerCb, &t, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);
	p->RegisterEvent(timerEv);

	p->EventLoop();

	delete p;
	delete s;
	delete ev;
	delete timerEv;

	//---------net test end-----------//

	return 0;
}