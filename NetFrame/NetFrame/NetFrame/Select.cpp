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

namespace NetFrame
{

	//Select& Select::Instance()
	//{
	//	static Select s_inst;
	//	return s_inst;
	//}


	Select::Select()
	{
		FD_ZERO(&m_rfds);
		FD_ZERO(&m_wfds);
		FD_ZERO(&m_efds);
	}


	Select::~Select()
	{
	}


	int Select::Init()
	{
		return 0;
	}


	int Select::Launch()
	{

		fd_set rfds = m_rfds;
		fd_set wfds = m_wfds;
		fd_set efds = m_efds;

		timeval val;
		val.tv_sec = 1;
		int cnt = select(MAX_FD, &rfds, &wfds, &efds, &val);

		if (0 > cnt)
		{
			printf("call select failed! code:%d\n", WSAGetLastError());
			return cnt;
		}

		if (0 == cnt)
			return 0;//continue;

		CollectEvent(rfds, wfds, efds);

		return 0;
	}


	void Select::CollectEvent(const fd_set& rfds, const fd_set& wfds, const fd_set& efds)
	{
		FdEvent fdEv;
		fdEv.ev = 0;

#ifdef _WIN32

		for (uint32 i = 0; i < rfds.fd_count; ++i)
		{
			fdEv.fd = rfds.fd_array[i];
			fdEv.ev = EV_IOREAD;

			PushActiveFd(fdEv);
		}

		for (uint32 i = 0; i < wfds.fd_count; ++i)
		{
			fdEv.fd = rfds.fd_array[i];
			fdEv.ev = EV_IOWRITE;

			PushActiveFd(fdEv);
		}

		for (uint32 i = 0; i < efds.fd_count; ++i)
		{
			fdEv.fd = rfds.fd_array[i];
			fdEv.ev = EV_IOEXCEPT;

			PushActiveFd(fdEv);
		}

#else
		std::set<socket_t> fds = *GetFds();
		for (auto it = fds.begin(); it != fds.end(); ++it)
		{
			socket_t fd = it->first;

			if (FD_ISSET(fd, &rfds))
				fdEv.ev |= EV_IOREAD;

			if (FD_ISSET(fd, &wfds))
				fdEv.ev |= EV_IOWRITE;

			if (FD_ISSET(fd, &efds))
				fdEv.ev |= EV_IOEXCEPT;

			else
				continue;

			fdEv.fd = fd;
			PushActiveFd(fdEv);
		}


#endif // _WIN32

	}


	void Select::RegistFd(socket_t fd, short ev)
	{
		if (ev & EV_IOREAD)
			FD_SET(fd, &m_rfds);
		if (ev & EV_IOWRITE)
			FD_SET(fd, &m_wfds);
		if (ev & EV_IOEXCEPT)
			FD_SET(fd, &m_efds);
	}


	void Select::CancelFd(socket_t fd)
	{
		FD_CLR(fd, &m_rfds);
		FD_CLR(fd, &m_wfds);
		FD_CLR(fd, &m_efds);
	}

}