/************C++ Source File****************
#
#	Filename: MyIOCP.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:47
#	Last Modified: 2018-08-11 17:40:47
*******************************************/

#ifdef _WIN32

#include "MyIOCP.h"

namespace NetFrame
{

	MyIOCP& MyIOCP::Instance()
	{
		static MyIOCP myIOCP_;
		return myIOCP_;
	}

}

#endif // !_WIN32
