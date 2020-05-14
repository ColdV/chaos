/************C++ Header File****************
#
#	Filename: NetDrive.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:39:58
#	Last Modified: 2018-08-11 17:39:58	
*******************************************/

#pragma once
#include "Socket.h"
#include <list>
#include <set>


namespace NetFrame
{

	struct FdEvent
	{
		socket_t fd;
		short ev;
	};

	class NetDrive
	{
	public:
		NetDrive();

		virtual ~NetDrive();

	public:
		virtual int Init() = 0;

		virtual int Launch() = 0;

		void AddFd(socket_t fd, uint32 ev) { m_fds.insert(fd); RegistFd(fd, ev); }

		void DelFd(socket_t fd) { m_fds.erase(fd); CancelFd(fd); }

		const std::list<FdEvent>& GetActives() const { return m_activeFd; }

		void ResetActives() { m_activeFd.clear(); }

		static NetDrive* AdapterNetDrive();


	protected:
		virtual void RegistFd(socket_t fd, short ev) {}

		virtual void CancelFd(socket_t fd) {}

		void PushActiveFd(const FdEvent& fdEv) { m_activeFd.push_back(fdEv); }

		std::set<socket_t>* GetFds() { return &m_fds; }
		

	protected:
		std::set<socket_t> m_fds;
		std::list<FdEvent> m_activeFd;
	};

}
