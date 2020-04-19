#pragma once


#include "stdafx.h"
#include "MyThread.h"

class MyWorkThread : public MyThread
{
public:
	MyWorkThread() {}
	~MyWorkThread() {}

	virtual void Run(); 
};