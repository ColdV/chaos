#pragma once

#include "stdafx.h"

class MutexGuard;
class Condition;

class Mutex : public NonCopyable
{
public:	
	friend MutexGuard;
	friend Condition;

	Mutex(/*int type = -1*/)
	{
#ifdef _WIN32
		//m_mutex = CreateMutex(NULL, FALSE, NULL);
		InitializeCriticalSection(&m_mutex);

#else
		//m_mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutexattr_t* mutexattr = NULL;

		//if (0 <= type)
		//{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			mutexattr = &attr;

			if (pthread_mutexattr_settype(mutexattr, PTHREAD_MUTEX_RECURSIVE/*type*/))
				return;
		//}

		pthread_mutex_init(&m_mutex, mutexattr);

#endif // _WIN32
	}


	~Mutex()
	{
#ifdef _WIN32
		//CloseHandle(m_mutex);
		DeleteCriticalSection(&m_mutex);
#else
		pthread_mutex_destroy(&m_mutex);
#endif // _WIN32
	}


	mutex_t* GetMutex() { return &m_mutex; }


private:
	int Lock()
	{
#ifdef _WIN32
		//return WaitForSingleObject(m_mutex, INFINITE);
		EnterCriticalSection(&m_mutex);
		return 0;
#else
		return pthread_mutex_lock(&m_mutex);
#endif // _WIN32
	}


	void UnLock()
	{
#ifdef _WIN32
		//ReleaseMutex(m_mutex);
		LeaveCriticalSection(&m_mutex);
#else
		pthread_mutex_unlock(&m_mutex);
#endif // _WIN32
	}


private:
	mutex_t m_mutex;

};


class MutexGuard : public NonCopyable
{
public:
	explicit MutexGuard(Mutex& mutex)
		:m_mutex(mutex)
	{
		m_mutex.Lock();
	}


	~MutexGuard()
	{
		m_mutex.UnLock();
	}

private:
	Mutex& m_mutex;
};