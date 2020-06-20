#pragma once

#include "../../common/stdafx.h"


class Sem :public NonCopyable
{
public:
	Sem()
	{
#ifdef WIN32
		m_sem = CreateSemaphore(NULL, 0, 1, NULL);
#endif // WIN32
	}


	~Sem()
	{
#ifdef WIN32
		CloseHandle(m_sem);
#endif // WIN32

	}

	sem_wait_ret SemWait()
	{
#ifdef WIN32
		return WaitForSingleObject(m_sem, INFINITE);
#endif // WIN32
	}


	int SemPost()
	{
#ifdef WIN32
		return ReleaseSemaphore(m_sem, 1, NULL);
#endif // WIN32
	}

private:
	sem_t m_sem;
};