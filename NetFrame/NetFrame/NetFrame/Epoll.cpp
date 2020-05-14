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


namespace NetFrame
{ 

	//Epoll& Epoll::Instance()
	//{
	//	static Epoll s_inst;
	//	return s_inst;
	//}

	Epoll::Epoll():
		m_epfd(0),
		m_events(0)
	{
	}


	Epoll:~Epoll()
	{
		if (m_events)
			delete[] m_events;
	}


	int Epoll::Init()
	{
		m_events = new epoll_event[MAX_FD];
		if (!m_events)
			return -1;

		m_epfd = epoll_create(MAX_FD);
		if (0 >= m_epfd)
			return m_epfd;

		return 0;
	}


	//void Epoll::WaitEvent()
	void Epoll::Launch()
	{
		int cnt = epoll_wait(m_epfd, m_events, MAX_FD, 1);
		if (0 > cnt)
			return -1;

		//IOEvent newEvent;
		FdEvent fdEv;

		for (int i = 0; i < cnt; ++i)
		{
		
			if (m_events[i].events & EPOLLIN)
				fdEv.ev = EV_IOREAD;

			else if (m_events[i].events & EPOLLOUT)
				fdEv.ev = EV_IOWRITE;

			else if (m_events[i].events & EPOLLERR)
				fdEv.ev = EV_IOEXCEPT;

			else
			{

				printf("unexpected event:%d\n", m_events[i].events);
				continue;
			}

			fdEv.fd = m_events[i].data.fd;
			/*addIOEvent(newEvent);*/
			PushActiveFd(fdEv);
		}

		return 0;
	}


	void Epoll::RegistFd(socket_t fd, short ev)
	{
		epoll_event epEv;
		epEv.data.fd = fd;
		epEv.events = EPOLLIN | EPOLLET;
		
		epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &epEv);
	}


	void Epoll::CancelFd(socket_t fd, short ev)
	{
		epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	}

}

#endif // !_WIN32
