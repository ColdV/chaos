#pragma once

#include "stdafx.h"

#ifdef _WIN32

typedef unsigned int (__stdcall *pThreadFunc)(void*);

#else

typedef	void (*pThreadFunc)(void*);

#endif // _WIN32



class MyThread
{
public:
	MyThread(): m_tid(0) {}
	virtual ~MyThread() {}

	virtual void Run() = 0;

	int Create(pThreadFunc func, void* arg);

	unsigned long int GetTid() const { return m_tid; }
	void SetTid(unsigned long int nTid) { m_tid = nTid; }

private:
	unsigned long int m_tid;
};
