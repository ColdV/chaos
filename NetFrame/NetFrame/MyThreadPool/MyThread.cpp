#include "MyThread.h"
#include "stdafx.h"

MyThread::MyThread()
{
	m_tid = 0;
	m_status = TS_INITIAL;
#ifdef _WIN32
	m_hThread = 0;
#endif
}


MyThread::~MyThread()
{

}



int MyThread::Start()
{
	if (TS_INITIAL != m_status)
		return -1;

#ifdef _WIN32

	m_hThread = (HANDLE)_beginthreadex(NULL, 0, MyThreadProcess, this, 0, (unsigned int*)&m_tid);

#else
	pthread_create(&m_tid, NULL, MyThreadProcess, this);

#endif // _WIN32

	SetStatus(TS_WAITING);

	return m_tid;
}


void MyThread::Stop()
{
#ifdef _WIN32
	int ret = TerminateThread(m_hThread, 0);
	printf("提前结束线程%d\n", ret);
#endif
}

#ifdef _WIN32

unsigned int __stdcall MyThread::MyThreadProcess(void* myThread)
{
	if (!myThread)
		return -1;

	MyThread* pThread = (MyThread*)myThread;

	if (!pThread)
	{
		printf("empty point thread\n");
		return -1;
	}

	//while (true)
	//{
		pThread->SetStatus(TS_RUNNING);
		pThread->Run();
		//pThread->SetStatus(TS_WAITING);
	//}
	pThread->SetStatus(TS_EXIT);
	return 0;
}

#else
void* MyThread::MyThreadProcess(void* myThread)
{
	if (!myThread)
		return 0;

	MyThread* pThread = (MyThread*)myThread;

	pThread->SetStatus(TS_RUNNING);
	pThread->Run();
	pThread->SetStatus(TS_EXIT);

	return 0;
}

#endif // _WIN32
