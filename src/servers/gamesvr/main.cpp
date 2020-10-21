#include <stdio.h>
#include "log/Logger.h"
#include "stdafx.h"
#include "net/Event.h"
#include <functional>
#include "common.h"
#include "net/Timer.h"

#ifdef _WIN32

#include <Psapi.h>
#include<crtdbg.h>

#pragma comment(lib,"psapi.lib")

#define _CRTDBG_MAP_ALLOC

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG


void ShowMemUse()
{
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	LOG_DEBUG("内存使用:%d", pmc.WorkingSetSize);
}

#endif // _WIN32


class Test
{
public:
	void ListenCb(chaos::Listener* ev, chaos::Connecter* ,/*socket_t fd,*/ void* userdata);

	void ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void TimerCb(chaos::Event* pev, short ev, void* userdata);
};


void Test::ListenCb(chaos::Listener* ev, chaos::Connecter* pConner, void* userdata)
{
	//ShowMemUse();
	//chaos::Connecter* pConner = new chaos::Connecter(fd);
	//ShowMemUse();

	if (!pConner)
	{
		printf("erro new conn!\n");
		return;
	}

	pConner->SetCallback(std::bind(&Test::ReadCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL,
		std::bind(&Test::WriteCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);

	LOG_DEBUG("socket:%d, newsocket:%d", ev->GetSocket().GetFd(), pConner->GetSocket().GetFd());
}


int cnt = 0;

void Test::ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	LOG_DEBUG("read socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);

	if (0 == nTransBytes)
	{
		ev->GetCentre()->CancelEvent(ev);
		return;
	}

	int readable = ev->GetReadableSize();
	char* buf = new char[readable];
	if (!buf)
		return;

	ev->ReadBuffer(buf, readable);
	ev->Write(buf, readable);
	
	

	LOG_DEBUG("read socket recv data:%s", buf);
	delete[] buf;

	//if(++cnt == 10000)
	//	ev->GetCentre()->Exit();
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

#ifdef _WIN32
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
	//_CrtSetBreakAlloc(151);		//270 152 151
#endif // _WIN32

	
	Logger& log = Logger::Instance();
	log.Init("./log", 0);

	chaos::EventCentre* p = new chaos::EventCentre;

	if (0 != p->Init())
	{
		printf("init event centre failed!\n");
		return 0;
	}

#if 0  //被CreateListener取代
	//socket_t listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//chaos::Listener* ev = new chaos::Listener(listenfd);

	//printf("listen socket:%d\n", listenfd);

	//sockaddr_in sa;
	//memset(&sa, 0, sizeof(sa));
	//sa.sin_family = AF_INET;
	//sa.sin_port = htons(PORT);
	////inet_pton(AF_INET, IP, &sa.sin_addr);

	//if (0 != ev->Listen((sockaddr*)&sa, sizeof(sa)))
	//{
	//	printf("listen failed! err:%d\n", WSAGetLastError());
	//	return -1;
	//}
#endif

	chaos::Listener* ev = chaos::Listener::CreateListener(AF_INET, SOCK_STREAM, IPPROTO_TCP, PORT);
	if (!ev)
	{
		printf("create listener failed!\n");
		return 0;
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

	delete p;
	//delete timerEv;

	//---------net test end-----------//

	return 0;
}