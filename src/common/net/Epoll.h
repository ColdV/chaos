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
			INIT_EVENT_LIST_SIZE = 64,
		};

		Epoll(EventCentre* pCentre);

		virtual ~Epoll();

		virtual int Init() override;

		virtual int Launch(int timeoutMs, Poller::EventList& activeEvents) override;

	protected:
		virtual int RegistFd(socket_t fd, short ev) override;

		virtual int CancelFd(socket_t fd, short ev) override;

	private:
		void SetEpollEvent(epoll_event& epev, socket_t fd, short ev);

	private:
		int m_epfd;
		epoll_event* m_epevs;
	};

}

#endif // !_WIN32
