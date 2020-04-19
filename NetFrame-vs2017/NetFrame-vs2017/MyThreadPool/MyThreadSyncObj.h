#pragma once

#include "stdafx.h"

class MyThreadSyncObj
{
public:
	MyThreadSyncObj() 
	{
		m_owner_thread = 0;
		m_error = 0;
	}

	virtual ~MyThreadSyncObj() {}

	unsigned long int GetOwner() const { return m_owner_thread; }
	void SetOwner(unsigned long int nTid) { m_owner_thread = nTid; }
	
	int GetError() const { return m_error; }
	void SetError(int errCode) { m_error = errCode; }

protected:
	unsigned long int m_owner_thread;
	int m_error;
};