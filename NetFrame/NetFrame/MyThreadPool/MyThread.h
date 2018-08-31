#pragma once

#include "stdafx.h"

#ifdef _WIN32

typedef unsigned int (__stdcall *pThreadFunc)(void*);

#else

typedef	void (*pThreadFunc)(void*);

#endif // _WIN32


enum ThreadStatus
{
	TS_INITIAL = 0,
	TS_RUNNING,
	TS_WAITING,
	TS_EXIT,
};

class MyThread
{
public:
	MyThread();
	virtual ~MyThread();

	virtual int Start();
	
	virtual void Stop() {}

	virtual void Run() = 0;

	unsigned long int GetTid() const { return m_tid; }

	void SetTid(unsigned long int nTid) { m_tid = nTid; }

	void SetStatus(ThreadStatus status) { m_status = status; }

	int GetStatus() const { return m_status; }

private:
#ifdef _WIN32
	static unsigned int __stdcall MyThreadProcess(void* myThread);
#else
	static void MyThreadProcess(void* myThread);
#endif // _WIN32

private:
	unsigned long int m_tid;
	ThreadStatus m_status;

#ifdef _WIN32
	HANDLE m_hThread;
#endif // _WIN32

};
