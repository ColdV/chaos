
#ifdef _WIN32
#include <process.h>
#endif // _WIN32

#include"../../common/template/MinHeap.h"
#include <map>
#include "Event.h"
#include "Timer.h"
#include "../../common/common.h"
#include "IOCP.h"
#include <Mswsock.h>
#include "../Thread/ThreadPool.h"
//#include "Poller.h"

#pragma comment(lib,"Mswsock.lib")
const char IP[] = "10.246.60.164";//"0.0.0.0";//"10.246.60.179";



#if 0
int main()
{
	printf("Listener:%d, Connecter:%d socket:%d, center:%d\n", sizeof(NetFrame::Listener), sizeof(NetFrame::Connecter),\
		sizeof(NetFrame::Socket), sizeof(NetFrame::EventCentre));

	NetFrame::EventCentre* pCentre = new NetFrame::EventCentre();

	if (!pCentre)
		return -1;

	if (0 != pCentre->Init())
		return -1;

	struct timeval timeout { 100, 0 };//{0, NetFrame::NET_TICK * 1000};

	NetFrame::Socket* s = new NetFrame::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, true);

	if (!s)
		return -1;

	s->Bind("192.168.0.101", 3307);
	s->Listen();

	NetFrame::EventKey key;
	key.fd = s->GetFd();

	NetFrame::Listener* netEv = new NetFrame::Listener(pCentre, s, EV_IOREAD| EV_IOEXCEPT, key);
	if (!netEv)
		return -1;



	pCentre->RegisterEvent(netEv);

	key.timerId = NetFrame::Timer::CreateTimerID();
	NetFrame::TimerEvent* ev = new NetFrame::TimerEvent(pCentre, EV_TIMEOUT, key, 1, true);

	//pCentre->RegisterEvent(ev);
	pCentre->EventLoop();

	return 0;
}

#endif




//------------threadpool test----------------

//#if 0

class Task : public ThreadTask
{
public:
	Task(int id) { m_id = id; }
	~Task() {}

	void Run()
	{
		//Sleep(3000);
		printf("i am task:%d!\n", m_id);
	}

private:
	int m_id;
};

int main()
{
	ThreadPool* pool = new ThreadPool();
	if (!pool)
		return -1;
	
	const int NUM = 1000;

	Task** all = new Task*[NUM];
	//Task* all[50];

	for (int i = 0; i < NUM; ++i)
	{
		all[i] = new Task(i);
	}

	pool->Run();


	Sleep(3000);

	for (int i = 0; i < NUM; ++i)
	{
		pool->PushTask(all[i]);
	}

	Sleep(3000);
	pool->Stop();

	while (1)
	{
		Sleep(1);
	}


	for (int i = 0; i < NUM; ++i)
		delete all[i];

	delete[] all;

	return 0;
}

//#endif