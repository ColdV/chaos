#pragma once


#include "../../common/stdafx.h"
#include "Mutex.h"

class Condition :public NonCopyable
{
public:
	Condition(Mutex& mutex):
		m_mutex(mutex)
	{
#ifdef WIN32
		m_cond = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
		pthread_cond_init(&m_cond, NULL);
#endif // WIN32
		
	}


	~Condition()
	{
#ifdef WIN32
		CloseHandle(m_cond);
#else
		pthread_cond_destroy(&m_cond);
#endif // WIN32

	}


	int CondWait(int timeOut = -1)
	{
#ifdef WIN32
		if (0 > timeOut)
			timeOut = INFINITE;

		//这里,先UnLock,后Lock,用来模拟pthread_cond_wait
		m_mutex.UnLock();
		int ret = WaitForSingleObject(m_cond, timeOut * 1000);
		if (0 == ret)
		{
			//由于创建Event时是手动模式,需要手动重置信号, 
			//否则所有线程的WaitForSingleObject会一直收到信号
			ResetEvent(m_cond);
		}

		m_mutex.Lock();

		return ret;
#else
		if (0 <= timeOut)
		{
			struct timespec timeSpec;
			timeSpec.tv_sec = timeOut;
			timeSpec.tv_usec = 0;
			
			return pthread_cond_timewait(&m_cond, m_mutex.GetMutex(), &timeSpec);
		}
		return pthread_cond_wait(&m_cond, m_mutex.GetMutex());
#endif // WIN32

	}


	int CondSignal()
	{
#ifdef WIN32
		SetEvent(m_cond);
		return ResetEvent(m_cond);
		//return SetEvent(m_cond);
#else
		return pthread_cond_signal(&m_cond);
#endif // WIN32
	}


	int CondBroadCast()
	{
#ifdef WIN32
		return PulseEvent(m_cond);
#else
		return pthread_cond_broadcast(&m_cond);
#endif // WIN32

	}

private:
	cond_t m_cond;
	Mutex& m_mutex;
};