#pragma once


#include "ThreadPool.h"
#include "../../common/stdafx.h"

class ThreadTask : public NonCopyable
{
public:
	ThreadTask():m_pArg(0) {}

	virtual ~ThreadTask()
	{
		if (m_pArg)
			delete m_pArg;
	}

	virtual void Run() = 0;

	const void* GetParam() const { return m_pArg; }
	void SetParam(void* arg) { m_pArg = arg; }

private:
	void* m_pArg;
};