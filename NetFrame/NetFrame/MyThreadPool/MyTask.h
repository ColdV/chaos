#pragma once
#include "MyThreadPool.h"

class MyTask
{
	friend MyThreadPool;

public:
	MyTask():m_task_param(0) {}

	virtual void Run() = 0;

	//void Destroy() { delete this; }

	void* GetParam() const { return m_task_param; }
	void SetParam(void* param) { m_task_param = param; }

protected:
	virtual ~MyTask() {}

private:
	void* m_task_param;
};