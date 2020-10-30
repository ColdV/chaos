#pragma once


#include "ThreadPool.h"
#include "stdafx.h"

class ThreadTask : public NonCopyable
{
public:
	ThreadTask():m_param(NULL) {}

	virtual ~ThreadTask() {}

	virtual void Run() = 0;

	const void* GetParam() const { return m_param; }
	void SetParam(void* param) { m_param = param; }

private:
	void* m_param;
};