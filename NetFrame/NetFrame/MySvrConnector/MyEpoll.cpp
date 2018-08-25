/************C++ Source File****************
#
#	Filename: MyEpoll.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:46
#	Last Modified: 2018-08-10 10:12:46
*******************************************/

#ifndef _WIN32

#include "MySocketIO.h"
#include "MyEpoll.h"

MyEpoll& MyEpoll::Instance()
{
	static MyEpoll myEpoll_;
	return myEpoll_;
}


#endif // !_WIN32
