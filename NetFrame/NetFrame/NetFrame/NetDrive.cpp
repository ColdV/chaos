/************C++ Source File****************
#
#	Filename: NetDrive.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:02
#	Last Modified: 2018-08-11 17:40:02
*******************************************/


#include "NetDrive.h"
#include "Select.h"
#include "Epoll.h"
#include "IOCP.h"

namespace NetFrame
{
	NetDrive::~NetDrive()
	{
		printf("delete IO!\n");
	}

	
	NetDrive* NetDrive::AdapterNetDrive()
	{
#ifdef _WIN32
		return &Select::Instance();
#else
		return &Epoll:Instance();
#endif

	}
}