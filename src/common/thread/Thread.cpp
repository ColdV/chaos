#include "Thread.h"
#include "stdafx.h"

Thread::Thread(PThreadFunc func, void* param /*= NULL*/) :
	m_tid(0),
	m_func(func),
	m_param(param),
	m_started(false),
	m_joined(false)
{
#ifdef _WIN32
	m_hThread = NULL;
#endif
}


Thread::~Thread()
{	
	if (m_started && !m_joined)
	{
		if (0 != Join())
		{
#ifdef _WIN32
			printf("wait thread end failed! force thread.%d\n", GetLastError());
			TerminateThread(m_hThread, -1);
			CloseHandle(m_hThread);
#else
			pthread_detach(m_tid);
#endif // _WIN32
		}
	}
}



int Thread::Start()
{
	if (!m_func || m_started)
		return -1;

#ifdef _WIN32

	m_hThread = (HANDLE)_beginthreadex(NULL, 0, m_func, m_param, 0, &m_tid);
	if (!m_hThread)
		return -1;

#else
	if (0 != pthread_create(&m_tid, NULL, m_func, m_param))
		return -1;
#endif // _WIN32

	m_started = true;

	return m_tid;
}


int Thread::Join()
{
	if (m_joined)
		return 0;

	m_joined = true;
	int ret = 0;
#ifdef _WIN32
	if (!m_hThread)
		return 0;

	ret = WaitForSingleObject(m_hThread, INFINITE);

	if (0 == ret)
		CloseHandle(m_hThread);
	else
		m_joined = false;
#else
	ret = pthread_join(m_tid, NULL);
	if (0 != ret)
		m_joined = false;

#endif // _WIN32

	return ret;
}
