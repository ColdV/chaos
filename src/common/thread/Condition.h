#pragma once


#include "stdafx.h"
#include "Mutex.h"

class Condition :public NonCopyable
{
public:
	enum CondState
	{
		CS_COND_NORMAL = 0,		
		CS_COND_BROADCAST,		//广播,唤醒等待该条件变量的所有线程
		CS_COND_SINGLE,			//唤醒单个线程
	};

	Condition(Mutex& mutex):
		m_mutex(mutex)
#ifdef _WIN32
		,m_waitThreadNum(0)
		,m_state(CS_COND_NORMAL)
#endif // _WIN32

	{
#ifdef _WIN32
		m_cond = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
		pthread_cond_init(&m_cond, NULL);
#endif // _WIN32
		
	}


	~Condition()
	{
#ifdef _WIN32
		CloseHandle(m_cond);
#else
		pthread_cond_destroy(&m_cond);
#endif // _WIN32

	}


	int CondWait(int timeoutMs = -1)
	{
#ifdef _WIN32
		bool forever = false;
		if (0 > timeoutMs)
			timeoutMs = INFINITE;

		int ret = 0;

		//这里,先UnLock,后Lock,用来模拟pthread_cond_wait
		m_mutex.UnLock();

		do
		{
			++m_waitThreadNum;

			ret = WaitForSingleObject(m_cond, timeoutMs);

			--m_waitThreadNum;

			if (CS_COND_SINGLE == m_state ||
				(CS_COND_BROADCAST == m_state && 0 == m_waitThreadNum)
				)
			{
				ResetEvent(m_cond);
				m_state = CS_COND_NORMAL;
			}

			//永久阻塞 类似于linux pthread_cond_wait
			if (WAIT_TIMEOUT == ret && INFINITE == timeoutMs)
				forever = true;

		} while (forever);

		m_mutex.Lock();

		return ret;
#else
		if (timeoutMs >= 0)
		{
			struct timespec timeSpec;
			timeSpec.tv_sec = timeoutMs / SEC2MSEC;
			timeSpec.tv_nsec = (timeoutMs % SEC2MSEC) * SEC2MSEC * SEC2MSEC;
			
			return pthread_cond_timedwait(&m_cond, m_mutex.GetMutex(), &timeSpec);
		}
		return pthread_cond_wait(&m_cond, m_mutex.GetMutex());
#endif // _WIN32

	}


	int CondSignal()
	{
#ifdef _WIN32
		//当前处于广播状态,所有等待中的线程都会被唤醒
		//没有必要再发送这次的单次信号
		if (CS_COND_BROADCAST == m_state)	
			return 0;

		m_state = CS_COND_SINGLE;

		return SetEvent(m_cond);
#else
		return pthread_cond_signal(&m_cond);
#endif // _WIN32
	}


	int CondBroadCast()
	{
#ifdef _WIN32
		m_state = CS_COND_BROADCAST;
		return SetEvent(m_cond);
#else
		return pthread_cond_broadcast(&m_cond);
#endif // _WIN32

	}

private:
	cond_t m_cond;

	Mutex& m_mutex;							//外部与该条件变量关联的锁

#ifdef _WIN32
	std::atomic<int> m_waitThreadNum;		//等待该条件变量的线程数量

	std::atomic<short> m_state;
#endif // _WIN32
};