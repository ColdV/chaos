#include "Thread.h"
#include "stdafx.h"

Thread::Thread(PThreadFunc func, void* param) :
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
	if (m_param)
		delete m_param;
	
	if (m_started && !m_joined)
	{
#ifdef _WIN32
		if(m_hThread)
			CloseHandle(m_hThread);
#else
		pthread_detach(m_tid);
#endif // _WIN32
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
	m_joined = true;
#ifdef _WIN32
	int ret = WaitForSingleObject(m_hThread, INFINITE);
	if (0 == ret)
	{
		CloseHandle(m_hThread);
	}
	else
		m_joined = false;
	return ret;
#else
	return pthread_join(m_tid, NULL);
#endif // _WIN32
}
