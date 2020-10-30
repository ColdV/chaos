#pragma once

#include "stdafx.h"


class Sem :public NonCopyable
{
public:
	Sem()
	{
#ifdef _WIN32
		m_sem = CreateSemaphore(NULL, 0, 1, NULL);
#endif // _WIN32
	}


	~Sem()
	{
#ifdef _WIN32
		CloseHandle(m_sem);
#endif // _WIN32

	}

	sem_wait_ret SemWait()
	{
#ifdef _WIN32
		return WaitForSingleObject(m_sem, INFINITE);
#endif // _WIN32
	}


	int SemPost()
	{
#ifdef _WIN32
		return ReleaseSemaphore(m_sem, 1, NULL);
#endif // _WIN32
	}

private:
	sem_t m_sem;
};