/************C++ Source File****************
#
#	Filename: Epoll.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:46
#	Last Modified: 2018-08-10 10:12:46
*******************************************/

#ifndef _WIN32

#include "Epoll.h"


namespace chaos
{ 

	//Epoll& Epoll::Instance()
	//{
	//	static Epoll s_inst;
	//	return s_inst;
	//}

	Epoll::Epoll(EventCentre* pCentre):
		Poller(pCentre),
		m_epfd(0),
		m_evs(0)
	{
	}


	Epoll::~Epoll()
	{
		if (m_evs)
			delete[] m_evs;
	}


	int Epoll::Init()
	{
		m_evs = new epoll_event[MAX_FD];
		if (!m_evs)
			return -1;

		m_epfd = epoll_create(MAX_FD);
		if (0 >= m_epfd)
			return m_epfd;

		return 0;
	}


	int Epoll::Launch(int timeoutMs)
	{
		if (0 >= timeoutMs)
			timeoutMs = NET_TICK;

		int cnt = epoll_wait(m_epfd, m_evs, MAX_FD, timeoutMs);
		if (0 > cnt && errno != EINTR)
			return -1;

		for (int i = 0; i < cnt; ++i)
		{
			short ev = 0;
			if (m_evs[i].events & EPOLLIN)
				ev = EV_IOREAD;

			else if (m_evs[i].events & EPOLLOUT)
				ev = EV_IOWRITE;

			else if (m_evs[i].events & EPOLLERR)
				ev = EV_IOEXCEPT;

			else
			{
				printf("unexpected event:%d\n", m_evs[i].events);
				continue;
			}

			PushActiveEvent(m_evs[i].data.fd, ev);
		}

		return 0;
	}


	int Epoll::RegistFd(socket_t fd, short ev)
	{
		epoll_event epev;
		memset(&epev, 0, sizeof(epoll_event));

		epev.data.fd = fd;

		Event* pEvent = GetEvent(fd);
		if (!pEvent)
			return -1;

		epev.events = pEvent->GetEv();

		if (ev & EV_IOREAD)
			epev.events |= EPOLLIN;
		if (ev & EV_IOWRITE)
			epev.events |= EPOLLOUT;

		epev.events |= DEFAULT_EPOLL_EVENTS;		// | EPOLLHUP;
		
		if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &epev) == 0)
			return 0;

		if (errno == ENOENT && epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &epev) == -1)
			return -1;

		return -1;
	}


	int Epoll::CancelFd(socket_t fd, short ev)
	{
		//epoll_event epev;
		//memset(&epev, 0, sizeof(epoll_event));

		//epev.data.fd = fd;

		//if (ev & EV_IOREAD)
		//	epev.events |= EPOLLIN;
		//if (ev & EV_IOWRITE)
		//	epev.events |= EPOLLOUT;

		//if (epev.events & EPOLLIN && epev.events & EPOLLOUT)
		//	epev.events |= DEFAULT_EPOLL_EVENTS;

		return epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	}

}

#endif // !_WIN32
