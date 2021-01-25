/************C++ Source File****************
#
#	Filename: Select.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:25
#	Last Modified: 2018-08-11 17:40:25
*******************************************/


#include "Select.h"
#include "Event.h"

namespace chaos
{
	Select::Select(EventCentre* pCentre):
		Poller(pCentre)
#ifndef _WIN32
		,m_maxfd(0)
#endif // !_WIN32
	{
		FD_ZERO(&m_rfds);
		FD_ZERO(&m_wfds);
		FD_ZERO(&m_efds);

		FD_ZERO(&m_rfdsout);
		FD_ZERO(&m_wfdsout);
		FD_ZERO(&m_efdsout);
	}


	Select::~Select()
	{

	}


	int Select::Init()
	{
		return 0;
	}


	int Select::Launch(int timeoutMs, EventList& activeEvents)
	{
		if (0 > timeoutMs)
			timeoutMs = -1;

		int nfds = 0;

#ifdef _WIN32
		//windows中的select调用不允许传入空的fd_set
		if (0 >= (m_rfds.fd_count + m_wfds.fd_count + m_efds.fd_count))
		{
			Sleep(timeoutMs);
			return 0;
		}

		nfds = m_rfds.fd_count > m_wfds.fd_count ? m_rfds.fd_count : m_wfds.fd_count;
		nfds = nfds > (int)m_efds.fd_count ? nfds : (int)m_efds.fd_count;
#else

#endif // _WIN32

		m_rfdsout = m_rfds;
		m_wfdsout = m_wfds;
		m_efdsout = m_efds;

		timeval val{ timeoutMs / SEC2MSEC, timeoutMs % SEC2MSEC * SEC2MSEC };

		int cnt = select(MAX_FD, &m_rfdsout, &m_wfdsout, &m_efdsout, &val);

		if (0 > cnt)
		{
			printf("call select failed! code:%d\n", GetLastErrorno());
#ifndef _WIN32
			if (GetLastErrorno() != EINTR)
				return cnt;
#else
			return cnt;
#endif // !_WIN32
		}

		else if (0 == cnt)
			return 0;//continue;

		CollectEvent(m_rfdsout, m_wfdsout, m_efdsout, activeEvents);

		return 0;
	}


	void Select::CollectEvent(const fd_set& rfds, const fd_set& wfds, const fd_set& efds, EventList& activeEvents)
	{
#ifdef _WIN32

		for (uint32 i = 0; i < rfds.fd_count; ++i)
		{
			PushActiveEvent(rfds.fd_array[i], EV_IOREAD, activeEvents);
		}

		for (uint32 i = 0; i < wfds.fd_count; ++i)
		{
			PushActiveEvent(wfds.fd_array[i], EV_IOWRITE, activeEvents);
		}

		for (uint32 i = 0; i < efds.fd_count; ++i)
		{
			PushActiveEvent(efds.fd_array[i], EV_IOEXCEPT, activeEvents);
		}

#else
		auto& evs = GetAllEvents();
		for (auto it = evs.begin(); it != evs.end(); ++it)
		{
			socket_t fd = it->first;
			short ev = 0;

			if (FD_ISSET(fd, &rfds))
				ev |= EV_IOREAD;

			if (FD_ISSET(fd, &wfds))
				ev |= EV_IOWRITE;

			if (FD_ISSET(fd, &efds))
				ev |= EV_IOEXCEPT;

			if (0 != ev)
				PushActiveEvent(fd, ev, activeEvents);
		}

#endif // _WIN32

	}


	int Select::RegistFd(socket_t fd, short ev)
	{
#ifndef _WIN32
		if (m_maxfd < fd)
			m_maxfd = fd;
#endif // !_WIN32

		if (ev & EV_IOREAD)
			FD_SET(fd, &m_rfds);
		if (ev & EV_IOWRITE)
			FD_SET(fd, &m_wfds);
		if (ev & EV_IOEXCEPT)
			FD_SET(fd, &m_efds);

		return 0;
	}


	int Select::CancelFd(socket_t fd, short ev)
	{
#ifndef _WIN32
		if (m_maxfd < fd)
			return 0;

		if (m_maxfd == fd)
			m_maxfd -= 1;
#endif // !_WIN32

		if(ev & EV_IOREAD)
			FD_CLR(fd, &m_rfds);
		if (ev & EV_IOWRITE)
			FD_CLR(fd, &m_wfds);
		if (ev & EV_IOEXCEPT)
			FD_CLR(fd, &m_efds);

		return 0;
	}

}