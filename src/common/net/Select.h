/************C++ Header File****************
#
#	Filename: Select.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:22
#	Last Modified: 2018-08-11 17:40:22
*******************************************/

#pragma once

#include "Poller.h"
#include "IOCP.h"

extern socket_t listenS;

namespace chaos
{
	class Select : public Poller
	{
	public:
		enum
		{
			MAX_FD = 1024,
		};

		//static Select& Instance();
		Select(EventCentre* pCentre);

		virtual ~Select();

		virtual int Init() override;

		virtual int Launch(int timeoutMs) override;

	protected:
		virtual int RegistFd(socket_t fd, short ev) override;
		
		virtual int CancelFd(socket_t fd, short ev) override;

	private:
		void CollectEvent(const fd_set& rfds, const fd_set& wfds, const fd_set& efds);

	private:
		fd_set m_rfds;
		fd_set m_wfds;
		fd_set m_efds;

		fd_set m_rfdsout;
		fd_set m_wfdsout;
		fd_set m_efdsout;

#ifndef _WIN32
		int m_maxfd;
#endif // !_WIN32


	};
}