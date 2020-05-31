#pragma once

#include "../../common/stdafx.h"

#ifdef WIN32
#define THREAD_FUNCTION_PRE unsigned int __stdcall
#else
#define THREAD_FUNCTION_PRE void
#endif // WIN32



#ifdef _WIN32
typedef unsigned int (__stdcall *PThreadFunc)(void*);
#else
typedef	void (*PThreadFunc)(void*);
#endif // _WIN32

class Thread : public NonCopyable
{
public:
	Thread(PThreadFunc func, void* arg);
	~Thread();

	int Start();

	int Join();

	thread_t GetTid() const { return m_tid; }

#ifdef _WIN32
	HANDLE GetThread() const { return m_hThread; }
#endif

private:
	thread_t m_tid;
	PThreadFunc m_func;
	void* m_param;
	bool m_started;
	bool m_joined;

#ifdef _WIN32
	HANDLE m_hThread;
#endif // _WIN32

};
