#ifndef _WIN32

#include "MyCondition.h"

MyCondition::MyCondition()
{
	m_cond = PTHREAD_COND_INITIALIZER;
}

MyCondition::~MyCondition()
{
	CondDestroy();
}

int MyCondition::CondInit(const pthread_condattr_t* attr)
{
	m_error = pthread_cond_init(&m_cond, attr);
	return m_error;
}

int MyCondition::CondDestroy()
{
	m_error = pthread_cond_destroy(&m_cond);
	return m_error;
}

int MyCondition::CondWait(MyMutex& mutex)
{
	m_error = pthread_cond_wait(&m_cond, &mutex.m_mutex);
	return m_error;
}

int MyCondition::CondTimeWait(MyMutex& mutex, const timespec* abstime)
{
	m_error = pthread_cond_timedwait(&m_cond, &mutex.m_mutex, abstime);
	return m_error;
}

int MyCondition::CondSignal()
{
	m_error = pthread_cond_signal(&m_cond);
	if (0 != m_error)
		return m_error;
	
	m_owner_thread = pthread_self();
	return m_error;
}

int MyCondition::CondBroadCast()
{
	m_error = pthread_cond_broadcast(&m_cond);
	if (0 != m_error)
		return m_error;

	m_owner_thread = pthread_self();
	return m_error;
}



#endif // !_WIN32
