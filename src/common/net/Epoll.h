/************C++ Header File****************
#
#	Filename: Epoll.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:31
#	Last Modified: 2018-08-10 10:12:31
*******************************************/

#ifndef _WIN32

#pragma once

#include "Poller.h"
#include "Event.h"
#include <sys/epoll.h>

const int DEFAULT_EPOLL_EVENTS = EPOLLET;

namespace chaos
{

	class Epoll : public Poller
	{
	public:
		enum
		{
			MAX_FD = 1024,
		};

		//static Epoll& Instance();
		Epoll(EventCentre* pCentre);

		virtual ~Epoll();

		virtual int Init();

		virtual int Launch(int timeoutMs) override;

	protected:
		virtual int RegistFd(socket_t fd, short ev);

		virtual int CancelFd(socket_t fd, short ev);

	private:
		int m_epfd;
		epoll_event* m_evs;
	};

}

#endif // !_WIN32
