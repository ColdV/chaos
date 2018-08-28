#pragma once

#ifdef _WIN32

#include "stdafx.h"
#include "MyThreadSyncObj.h"

class MyEvent : public MyThreadSyncObj
{
public:
	MyEvent();
	MyEvent(HANDLE hEvent, char* pName = NULL, int nNameSize = 0);
	~MyEvent();

	int CreateEvent(bool isReset, bool initState, char* pName = NULL, int nNameSize = 0);
	bool SetEvent();
	bool ResetRevent();
	int WaitEvent(DWORD dwMilliseconds = INFINITE);

	HANDLE GetEvent() const { return m_event; }
	const char* GetEventName() const { return m_name; }

private:
	HANDLE m_event;
	char m_name[32];
};


#endif // _WIN32
