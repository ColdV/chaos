#include "MyThread.h"


MyThread::MyThread()
{
	m_tid = 0;
	m_hThread = 0;
	m_status = TS_INITIAL;
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


#ifdef _WIN32

unsigned int __stdcall MyThread::MyThreadProcess(void* myThread)
{
	if (!myThread)
		return -1;

	MyThread* pThread = (MyThread*)myThread;

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
void MyThread::MyThreadProcess(void* myThread)
{
	if (!myThread)
		return;

	MyThread* pThread = (MyThread*)myThread;

	pThread->SetStatus(TS_RUNNING);
	pThread->Run();
	pThread->SetStatus(TS_EXIT);
}

#endif // _WIN32
