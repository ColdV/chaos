#pragma once

#include "stdafx.h"

class MyThread
{
public:
	MyThread(): m_tid(0) {}
	virtual ~MyThread() {}

	virtual void Run() = 0;

	unsigned long int GetTid() const { return m_tid; }
	void SetTid(unsigned long int nTid) { m_tid = nTid; }

private:
	unsigned long int m_tid;
};
