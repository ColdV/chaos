#pragma once
#include "MyThreadPool.h"

class MyTask
{
	friend MyThreadPool;

public:
	MyTask() {}

	virtual void Run() = 0;

	//void Destroy() { delete this; }

protected:
	virtual ~MyTask() {}
};