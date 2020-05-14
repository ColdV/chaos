/************C++ Header File****************
#
#	Filename: IOCP.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:43
#	Last Modified: 2018-08-11 17:40:43
*******************************************/
#pragma once


#ifdef _WIN32

#include "NetDrive.h"

namespace NetFrame
{

	class IOCP : public NetDrive
	{
	public:
		//static IOCP& Instance();
		IOCP() {}
		~IOCP() {}

		virtual int Init() {};

		virtual int Launch() {};
	};

}


#endif // _WIN32
