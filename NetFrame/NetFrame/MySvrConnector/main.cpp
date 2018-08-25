#include "MySvrConnector.h"
#include "MySocketIO.h"


int main()
{
	/*
	MySvrConnector svr(FD_SETSIZE);
	svr.run();
	return 0;
	*/

	
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
	

	return 0;
}