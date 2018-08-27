#include "MyThread.h"

int MyThread::Create(pThreadFunc func, void* arg)
{
	int res = 0;
#ifdef _WIN32
	//_beginthreadex_proc_type

	res = _beginthreadex(NULL, 0, func, arg, 0, (unsigned int*)&m_tid);
	

#else
	res = pthread_create(&m_tid, NULL, func, arg);

#endif // _WIN32
	return res;
}