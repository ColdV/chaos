#include "MyThread.h"

int MyThread::Start()
{
#ifdef _WIN32

	  m_hThread = (HANDLE)_beginthreadex(NULL, 0, MyThreadProcess, this, 0, (unsigned int*)&m_tid);
	  return m_tid;

#else
	return pthread_create(&m_tid, NULL, func, arg);

#endif // _WIN32
}


#ifdef _WIN32

unsigned int __stdcall MyThread::MyThreadProcess(void* myThread)
{

}

#else
void MyThread::MyThreadProcess(void* myThread)
{

}

#endif // _WIN32
