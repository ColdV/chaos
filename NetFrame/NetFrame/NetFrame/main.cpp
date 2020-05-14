
#include "../common/single_templete.h"
#include "../MyThreadPool/MyThreadPool.h"

#ifdef _WIN32
#include <process.h>
#endif // _WIN32

#include "MinHeap.h"
#include <map>
#include "Event.h"
#include "Timer.h"


const char IP[] = "10.246.60.164";//"0.0.0.0";//"10.246.60.179";

/*
void func(void* tid)
{
	int i = *(int*)tid;
	printf("i am thread:%d\n", GetCurrentThreadId());
	return;
}
*/

class test : public SingleTmp<test>
{
public:
	//test() {}
	~test() {}
	void print() { printf("single test!\n"); }
	test() {}
};


//template<class T = void>
//void print(T a)
//{
//	if (T == void)
//	{
//		printf("non T!\n");
//		return;
//
//	}
//
//	printf("%d\n", a);
//}

void timecb(unsigned int timeid, void* userData)
{
	printf("timeid:%u, time:%lld\n", timeid, time(NULL));
}


class TestNonCpy
{
public:
	TestNonCpy(const TestNonCpy&) = delete;
	void operator=(TestNonCpy&) = delete;
	TestNonCpy(int value) :m_value(value)
	{}
	~TestNonCpy() {}

private:
	int m_value;
};

void testNonCpy(TestNonCpy cpy)
{
	;
}
#include "../common/common.h"


#if 0
int main()
{

	/*TestNonCpy c(1);
	testNonCpy(c);*/
	//SetConsoleTitle(L"NetFrame");
	/*
	MySvrConnector svr(FD_SETSIZE);
	svr.run();
	return 0;
	*/

	//test t1 = test::Instance();
	//t1.print();

//#ifndef _WIN32
	//if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	//   return -1;
//#endif
	//NetDrive* p = CreateSocketIO(FD_SETSIZE, SI_EPOLL);

	//if (!p)
	//{
	//	printf("no found suitable IO\n");
	//	return 0;
	//}

	//p->InitIO(IP, 6666, FD_SETSIZE);

	//while (true)
	//{
	//	p->WaitEvent();

	//	if(0 < p->GetEventSize())
	//		printf("wait event!event size:%d\n", p->GetEventSize());

	//	
	//	while (!p->EventEmpty())
	//	{
	//		p->HandleEvent(p->GetIOEvent());
	//		printf("handle event! event size:%d\n", p->GetEventSize());
	//	}
	//	
	//}



	//EventMaster em;
	//em.Init();
	//em.Loop();


/*
	MyThreadPool pool(4);

	int i = 0;
	Sleep(5000);

	while (true)
	{
		printf("running\n");
	}
*/

	//typedef void (*TimerCallback)(unsigned int timeID, void* userData);


	//struct TimeEv
	//{
	//	unsigned int timeID;
	//	time_t dispatchTime;
	//	TimerCallback cb;
	//};


	//struct TimeEvCmp
	//{
	//	bool operator ()(TimeEv& left, TimeEv& right) const
	//	{
	//		return left.dispatchTime < right.dispatchTime;
	//	}
	//};

	//MinHeap<TimeEv, TimeEvCmp> minHeap;
	//srand(time(NULL));

	//TimeEv t;
	//t.timeID = 1;
	//t.dispatchTime = 3;// +time(NULL);
	//t.cb = timecb;

	//minHeap.Push(&t);

	//time_t lastTime = time(NULL);
	//while (true)
	//{
	//	while(minHeap.Top()->dispatchTime <= time(NULL) - lastTime)
	//	//if (minHeap.Top()->dispatchTime <= time(NULL))
	//	{
	//		TimeEv* pT = minHeap.Pop();
	//		pT->cb(pT->timeID, NULL);
	//		++pT->timeID;
	//		minHeap.Push(pT);
	//		lastTime = time(NULL);
	//	}

	//	Sleep(minHeap.Top()->dispatchTime * 1000);

	//	//lastTime = time(NULL);
	//}
	

	//MinHeap<int> min;

	//const int SIZE = 100;

	//int ary[100];// = { 5,0,7,8,4,9,1,3,2,6 };

	//for (int i = 0; i < SIZE; ++i)
	//{
	//	ary[i] = rand();
	//}

	//for (int i = 0; i < SIZE; ++i)
	//{
	//	min.Push(ary[i]);
	//}

	//printf("size:%d\n", min.GetSize());


	//for (int i = 0; i < SIZE; ++i)
	//{
	//	printf("pop:%d\n", *min.Top());
	//	min.Pop();
	//}

	//printf("size:%d\n", min.GetSize());

//int a = 8;
//char b[10] = { 1 };
//SetBit(a, 1);
//printf("%d\n", a);
//ClrBit(a, 1);
//printf("%u\n", b[0]);
//	return 0;
}

#endif


int main()
{
	NetFrame::EventCentre* pCentre = new NetFrame::EventCentre();
	if (!pCentre)
		return -1;

	if (0 != pCentre->Init())
		return -1;
	
	socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	NetFrame::Socket* s = new NetFrame::Socket(sock);
	if (!s)
		return -1;

	s->Bind("0.0.0.0", 3307);
	s->Listen();

	NetFrame::EventKey* pNKey = new NetFrame::EventKey();
	pNKey->fd = sock;

	NetFrame::NetEvent* netEv = new NetFrame::NetEvent(pCentre, s, EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT, pNKey);
	if (!netEv)
		return -1;

	pCentre->RegisterEvent(netEv);

	NetFrame::EventKey* pKey = new NetFrame::EventKey();
	pKey->timerId = NetFrame::Timer::CreateTimerID();
	NetFrame::TimerEvent* ev = new NetFrame::TimerEvent(pCentre, EV_TIMEOUT, pKey, 1, true);

	pCentre->RegisterEvent(ev);
	pCentre->EventLoop();

	return 0;
}

