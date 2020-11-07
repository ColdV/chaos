#pragma once

#include "stdafx.h"

#ifndef _WIN32
#include <semaphore.h>
#endif //_WIN32

class Sem :public NonCopyable
{
public:
	Sem()
	{
#ifdef _WIN32
		m_sem = CreateSemaphore(NULL, 0, 1, NULL);
#else
        sem_init(&m_sem, 0, 0);
#endif // _WIN32
	}


	~Sem()
	{
#ifdef _WIN32
		CloseHandle(m_sem);
#else
        sem_destroy(&m_sem);
#endif // _WIN32

	}

	sem_wait_ret SemWait()
	{
#ifdef _WIN32
		return WaitForSingleObject(m_sem, INFINITE);
#else
        return sem_wait(&m_sem);
#endif // _WIN32
	}


	int SemPost()
	{
#ifdef _WIN32
		return ReleaseSemaphore(m_sem, 1, NULL);
#else
        return sem_post(&m_sem);
#endif // _WIN32
	}

private:
	sem_t m_sem;
};
