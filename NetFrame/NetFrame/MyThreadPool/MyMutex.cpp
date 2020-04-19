#include "MyMutex.h"

MyMutex::MyMutex()
{
	m_owner_thread = 0;

#ifdef _WIN32
	m_mutex = CreateMutex(NULL, 0, NULL);

#else
	m_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif // _WIN32
}


MyMutex::~MyMutex()
{
	Destroy();
}

void MyMutex::Lock()
{
#ifdef _WIN32
	WaitForSingleObject(m_mutex, INFINITE);
	m_owner_thread = GetCurrentThreadId();
#else
	pthread_mutex_lock(&m_mutex);
	m_owner_thread = pthread_self();
#endif // _WIN32
}


void MyMutex::UnLock()
{
#ifdef _WIN32
	if (ReleaseMutex(m_mutex))
		m_owner_thread = 0;
#else
	if (0 == pthread_mutex_unlock(&m_mutex))
		m_owner_thread = 0;
#endif // _WIN32
}


void MyMutex::Destroy()
{
#ifdef _WIN32
	CloseHandle(m_mutex);
#else
	pthread_mutex_destroy(&m_mutex);
#endif // _WIN32
}
