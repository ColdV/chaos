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
		m_epevs(0)
	{
	}


	Epoll::~Epoll()
	{
		if (m_epevs)
			delete[] m_epevs;
	}


	int Epoll::Init()
	{
		m_epevs = new epoll_event[MAX_FD];
		if (!m_epevs)
			return -1;

		m_epfd = epoll_create(MAX_FD);
		if (0 >= m_epfd)
			return m_epfd;

		return 0;
	}


	int Epoll::Launch(int timeoutMs, Poller::EventList& activeEvents)
	{
		if (0 > timeoutMs)
			timeoutMs = -1;

		int cnt = epoll_wait(m_epfd, m_epevs, MAX_FD, timeoutMs);
		if (0 > cnt && errno != EINTR)
			return -1;

		for (int i = 0; i < cnt; ++i)
		{
			short ev = 0;
			if (m_epevs[i].events & EPOLLIN)
				ev |= EV_IOREAD;

			if (m_epevs[i].events & EPOLLOUT)
				ev |= EV_IOWRITE;

			if (m_epevs[i].events & EPOLLERR)
				ev |= EV_IOEXCEPT;

			PushActiveEvent(m_epevs[i].data.fd, ev, activeEvents);
		}

		return 0;
	}


	int Epoll::RegistFd(socket_t fd, short ev)
	{
		Event* pEvent = GetEvent(fd);
		if (!pEvent)
			return -1;

		int curev = pEvent->GetEv() | ev;
		epoll_event epev;
		
		SetEpollEvent(epev, fd, curev);

		if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &epev) == 0)
			return 0;

		//如果fd已经注册到epoll中,上一次的epoll_ctl将返回ENOENT错误
		//将忽略该错误 并用EPOLL_CTL_MOD再次调用epoll_ctl以修改已注册的fd事件
		if (errno == ENOENT && epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &epev) == -1)
			return -1;

		return -1;
	}


	int Epoll::CancelFd(socket_t fd, short ev)
	{
		Event* pEvent = GetEvent(fd);
		if(!pEvent)
			return epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);

		int curev = pEvent->GetEv() & ~ev;
		if (curev & (EV_IOREAD | EV_IOWRITE))
		{
			epoll_event epev;
			
			SetEpollEvent(epev, fd, curev);

			return epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &epev);
		}

		return epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	}


	void Epoll::SetEpollEvent(epoll_event& epev, socket_t fd, short ev)
	{
		memset(&epev, 0, sizeof(epoll_event));

		epev.data.fd = fd;

		if (ev & EV_IOREAD)
			epev.events |= EPOLLIN;
		if (ev & EV_IOWRITE)
			epev.events |= EPOLLOUT;

		epev.events |= DEFAULT_EPOLL_EVENTS;	// | EPOLLHUP;
	}

}

#endif // !_WIN32
