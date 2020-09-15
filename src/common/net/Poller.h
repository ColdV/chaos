/************C++ Header File****************
#
#	Filename: Poller.h
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
#include "../thread/Mutex.h"

namespace chaos
{

	class Event;
	class EventCentre;

	const int NET_TICK = 1;	// 1000 / 60;		//“ª√Î60÷°

	class Poller : public NonCopyable
	{
	public:
		typedef std::map<socket_t, Event*> NetEventMap;

		Poller(EventCentre* pCentre);

		virtual ~Poller();

	public:
		virtual int Init() = 0;

		virtual int Launch(int timeoutMs) = 0;

		int AddEvent(Event* pEvent);

		int DelEvent(Event* pEvent);

		int DelEvent(socket_t fd);

		Event* GetEvent(socket_t);

		//mutex_lock_ret Lock() { return m_mutex.Lock(); }

		//void UnLock() { return m_mutex.UnLock(); }

		EventCentre& GetCentre() const { return *m_pCentre; }

		static Poller* AdapterNetDrive(EventCentre* pCentre);


	protected:
		virtual int RegistFd(socket_t fd, short ev) { return 0; }

		virtual int RegistFd(const Event* pEvent) { return 0; }

		virtual int CancelFd(socket_t fd) { return 0; }

		int PushActiveEvent(socket_t fd, short ev);

		int PushActiveEvent(Event* pEvent);

		const NetEventMap& GetAllEvents() const { return m_events; }

	private:
		Poller(const Poller&);

	protected:
		EventCentre* m_pCentre;
		//std::set<socket_t> m_fds;
		NetEventMap m_events;

		//Mutex m_mutex;
	};

}
