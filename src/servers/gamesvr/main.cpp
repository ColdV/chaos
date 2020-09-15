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
	void ListenCb(chaos::Listener* ev, /*chaos::Connecter**/socket_t fd, void* userdata);

	void ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void TimerCb(chaos::Event* pev, short ev, void* userdata);
};


void Test::ListenCb(chaos::Listener* ev, /*chaos::Connecter* pConner*/socket_t fd, void* userdata)
{
	chaos::Connecter* pConner = new(std::nothrow) chaos::Connecter(/*ev->GetCentre(),*/ fd);

	if (pConner)
	{
		//问题1:closesocket和closehandle重复调用会异常
		//问题2:delete掉Event后在EvQueue仍可能存在当前已发生的Event
		//问题4:在Event.cpp 410行 socket函数抛出异常 贼恶心
		//问题5:上面的那个new会抛异常  最他妈恶心 操
		//考虑1:在Centre的CancelEvent中设置回调通知用户释放Event
		pConner->SetCallback(std::bind(&Test::ReadCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL,
			std::bind(&Test::WriteCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);

		ev->GetCentre()->RegisterEvent(pConner);
	}

	LOG_DEBUG("socket:%d, newsocket:%d", ev->GetSocket().GetFd(), pConner->GetSocket().GetFd());
}


void Test::ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	LOG_DEBUG("read socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);
	int readable = ev->GetReadableSize();
	char* buf = new char[readable];
	if (!buf)
		return;

	ev->ReadBuffer(buf, readable);
	ev->Write(buf, readable);
	
	if (0 == nTransBytes)
		//delete ev;
		ev->GetCentre()->CancelEvent(ev);

	LOG_DEBUG("read socket recv data:%s", buf);
	delete[] buf;
}


void Test::WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	LOG_DEBUG("write socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);
}


void Test::TimerCb(chaos::Event* pev, short ev, void* userdata)
{
	LOG_DEBUG("timer:%d call back", pev->GetEvKey().timerId);
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

	Logger& log = Logger::Instance();
	log.Init("./log", 0);

	chaos::EventCentre* p = new chaos::EventCentre;

	p->Init();

	chaos::Socket* s = new chaos::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	chaos::Listener* ev = new chaos::Listener(/*p, */s->GetFd());

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

	//timer_id timerId = chaos::Timer::CreateTimerID();
	//chaos::TimerEvent* timerEv = new chaos::TimerEvent(p, timerId, 3, true);
	//timerEv->SetEventCallback(std::bind(&Test::TimerCb, &t, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);
	//p->RegisterEvent(timerEv);
	p->EventLoop();

	//socket_t fd = 1001;
	//CloseHandle((HANDLE)fd);

	delete p;
	delete s;
	delete ev;
	//delete timerEv;

	//---------net test end-----------//

	return 0;
}