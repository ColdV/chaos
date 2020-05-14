/************C++ Source File****************
#
#	Filename: IOCP.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:47
#	Last Modified: 2018-08-11 17:40:47
*******************************************/

#if 0

#ifdef _WIN32

#include "IOCP.h"

namespace NetFrame
{

	IOCP& IOCP::Instance()
	{
		static IOCP s_iocp;
		return s_iocp;
	}

}

#endif // !_WIN32

#endif