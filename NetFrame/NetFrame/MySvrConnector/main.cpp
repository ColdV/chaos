#include "MySvrConnector.h"
#include "MySocketIO.h"
#include "../common/single_templete.h"
#include "../MyThreadPool/MyThreadPool.h"

#ifdef _WIN32
#include <process.h>
#endif // _WIN32

#include "EventMaster.h"

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


int main()
{
	//SetConsoleTitle(L"NetFrame");
	/*
	MySvrConnector svr(FD_SETSIZE);
	svr.run();
	return 0;
	*/

	//test t1 = test::Instance();
	//t1.print();
	
#ifndef _WIN32
    //if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    //   return -1;
#endif

	
	//MySocketIO* p = CreateSocketIO(FD_SETSIZE, SI_EPOLL);

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


	
	EventMaster em;
	em.Init();
	em.Loop();


/*
	MyThreadPool pool(4);

	int i = 0;
	Sleep(5000);

	while (true)
	{
		printf("running\n");
	}
	*/
	return 0;
}
