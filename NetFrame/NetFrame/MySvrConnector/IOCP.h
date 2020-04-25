/************C++ Header File****************
#
#	Filename: IOCP.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:43
#	Last Modified: 2018-08-11 17:40:43
*******************************************/

#ifdef _WIN32

#pragma once

#include "NetDrive.h"

namespace NetFrame
{

	class IOCP : public NetDrive
	{
	public:
		static IOCP& Instance();
		~IOCP() {}

		virtual int InitIO(const char* ip, int port, uint32 max_fd) { return 0; }

		virtual void WaitEvent() {};

		virtual void HandleEvent(const IOEvent& fdEvent) {};

	protected:
		IOCP() {}
	};

}


#endif // _WIN32
