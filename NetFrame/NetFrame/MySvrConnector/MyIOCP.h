/************C++ Header File****************
#
#	Filename: MyIOCP.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:43
#	Last Modified: 2018-08-11 17:40:43
*******************************************/

#ifdef _WIN32

#pragma once

#include "MySocketIO.h"

class MyIOCP : public MySocketIO
{
public:
	static MyIOCP& Instance();
	~MyIOCP() {}

	virtual int InitIO(const char* ip, int port) { return 0; }

	virtual void WaitEvent() {};

	virtual void HandleEvent(const IOEvent& fdEvent) {};

protected:
	MyIOCP() {}
};

#endif // _WIN32