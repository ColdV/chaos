#pragma once

#include "../../common/stdafx.h"

class Mutex : public NonCopyable
{
public:	
	Mutex()
	{
#ifdef _WIN32
		m_mutex = CreateMutex(NULL, FALSE, NULL);

#else
		m_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif // _WIN32
	}


	~Mutex()
	{
#ifdef _WIN32
		CloseHandle(m_mutex);
#else
		pthread_mutex_destroy(&m_mutex);
#endif // _WIN32
	}


	mutex_t* GetMutex() { return &m_mutex; }

	mutex_lock_ret Lock()
	{
#ifdef _WIN32
		return WaitForSingleObject(m_mutex, INFINITE);
#else
		return pthread_mutex_lock(&m_mutex);
#endif // _WIN32
	}


	void UnLock()
	{
#ifdef _WIN32
		ReleaseMutex(m_mutex);
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