#pragma once

#ifndef _WIN32

#include "stdafx.h"
#include "MyMutex.h"

class MyCondition : public MyThreadSyncObj
{
public:
	MyCondition();
	~MyCondition();

	int CondInit(const pthread_condattr_t* attr);
	int CondDestroy();

	int CondWait(MyMutex& mutex);
	int CondTimeWait(MyMutex& mutex, const timespec* abstime);

	int CondSignal();
	int CondBroadCast();
	

private:
	pthread_cond_t m_cond;
};

#endif // !_WIN32