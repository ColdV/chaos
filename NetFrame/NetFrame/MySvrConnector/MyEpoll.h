/************C++ Header File****************
#
#	Filename: MyEpoll.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:31
#	Last Modified: 2018-08-10 10:12:31
*******************************************/

#ifndef _WIN32

#pragma once

#include "MySocketIO.h"

class MyEpoll : public MySocketIO
{
public:

	static MyEpoll& Instance();

	virtual ~MyEpoll() {}

	virtual int InitIO(const char* ip, int port) { return 0; }

	virtual void WaitEvent() {};

	virtual void HandleEvent(const IOEvent& fdEvent) {};

protected:
	MyEpoll() {}
};

#endif // !_WIN32