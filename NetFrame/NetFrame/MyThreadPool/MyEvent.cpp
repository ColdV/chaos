#ifdef _WIN32

#include "MyEvent.h"
#include "../common/common.h"

/*
MyEvent::MyEvent()
{
	m_event = NULL;
	memset(m_name, 0, sizeof(m_name));
}
*/

MyEvent::MyEvent(HANDLE hEvent, char* pName /*= NULL*/, int nNameSize /*= 0*/)
{
	m_event = hEvent;
	strncpy_safe(m_name, sizeof(m_name), pName, nNameSize);
}

MyEvent::MyEvent(bool isReset /*= false*/, bool initState /*= false*/, char* pName /*= NULL*/, int nNameSize /*= 0*/)
{
	if (!pName && nNameSize > sizeof(m_name))
		return;

	m_event = ::CreateEventA(NULL, isReset, initState, pName);
	strncpy_safe(m_name, sizeof(m_name), pName, nNameSize);
}


MyEvent::~MyEvent()
{
	CloseHandle(m_event);
}

int MyEvent::CreateEvent(bool isReset, bool initState, char* pName /*= NULL*/, int nNameSize /*= 0*/)
{
	if (!pName && nNameSize > sizeof(m_name))
		return -1;
	

	m_event = ::CreateEventA(NULL, isReset, initState, pName);
	strncpy_safe(m_name, sizeof(m_name), pName, nNameSize);

	return 0;
}

bool MyEvent::SetEvent()
{
	return ::SetEvent(m_event);
}

bool MyEvent::ResetRevent()
{
	return ::ResetEvent(m_event);
}

int MyEvent::WaitEvent(DWORD dwMilliseconds /*= INFINITE*/)
{
	return WaitForSingleObject(m_event, dwMilliseconds);
}

#endif
