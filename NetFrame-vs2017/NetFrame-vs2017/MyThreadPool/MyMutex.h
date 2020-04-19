#pragma once

#include "stdafx.h"
#include "MyThreadSyncObj.h"

class MyCondition;
class MyMutex : public MyThreadSyncObj
{
friend MyCondition;
public:	
	MyMutex();
	~MyMutex();

	void Lock();
	void UnLock();
	void Destroy();

private:

#ifdef _WIN32
	HANDLE m_mutex;
#else
	pthread_mutex_t m_mutex;
#endif // _WIN32
};
