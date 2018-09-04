#include "MySvrConnector.h"
#include "MySocketIO.h"
#include <process.h>
#include "../common/single_templete.h"
#include "../MyThreadPool/MyThreadPool.h"

void func(void* tid)
{
	int i = *(int*)tid;
	printf("i am thread:%d\n", GetCurrentThreadId());
	return;
}


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
	SetConsoleTitle(L"NetFrame");
	/*
	MySvrConnector svr(FD_SETSIZE);
	svr.run();
	return 0;
	*/

	//test t1 = test::Instance();
	//t1.print();
	
	/*
	MySocketIO* p = CreateSocketIO(FD_SETSIZE);

	if (!p)
	{
		printf("no found suitable IO\n");
		return 0;
	}

	p->InitIO("0.0.0.0", 6666);

	while (true)
	{
		p->WaitEvent();
		printf("wait event!event size:%d\n", p->GetEventSize());

		while (!p->EventEmpty())
		{
			p->HandleEvent(p->GetIOEvent());
			printf("handle event! event size:%d\n", p->GetEventSize());
		}
	}
	*/

	/*
	uintptr_t th[2] = { 0 };
	int prag[2] = { 1,2 };
	unsigned long int tid[2] = { 0 };
	th[0] = _beginthread(func, 0, (void*)&prag[0]);
	printf("_beginthread return:%d\n", th[0]);
	*/


	MyThreadPool pool(8);

	int i = 0;
/*
	while (true)
	{
		;
	}
*/
	return 0;
}