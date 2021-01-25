/************C++ Source File****************
#
#	Filename: Poller.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:02
#	Last Modified: 2018-08-11 17:40:02
*******************************************/


#include "Poller.h"
#include "Select.h"
#include "Epoll.h"
#include "IOCP.h"
#include "Event.h"

namespace chaos
{

	Poller::Poller(EventCentre* pCentre):
		m_pCentre(pCentre)
	{
	}


	Poller::~Poller()
	{
	}


	int Poller::AddEvent(const EventSharedPtr& pEvent)
	{
		if (!pEvent)
			return -1;

		const EventKey& key = pEvent->GetEvKey();
		
		if (m_events.find(key.fd) != m_events.end())
			return -1;

		m_events.insert(std::make_pair(key.fd, pEvent));

		if (0 != RegistFd(key.fd, pEvent->GetEv()))
		{
			m_events.erase(key.fd);
			return -1;
		}

		return 0;
	}

	
	int Poller::DelEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		CancelFd(pEvent->GetEvKey().fd, pEvent->GetEv());

		m_events.erase(pEvent->GetEvKey().fd);

		return 0;
	}


	void Poller::UpdateFd(socket_t fd, short op, short ev)
	{
		Event* pEvent = GetEvent(fd);
		if (!pEvent)
			return;

		if (EV_CTL_ADD == op)
			RegistFd(fd, ev);
		else if (EV_CTL_DEL == op)
			CancelFd(fd, ev);
	}


	Event* Poller::GetEvent(socket_t fd)
	{
		auto it = m_events.find(fd);

		if (it == m_events.end())
			return NULL;

		return it->second.get();
	}


	int Poller::PushActiveEvent(socket_t fd, short ev, EventList& activeEvents)
	{
		auto it = m_events.find(fd);

		if (it == m_events.end())
			return -1;

		if (!m_pCentre)
			return -1;
		
		EventSharedPtr& pEvent = it->second;

		if (!pEvent || pEvent->GetCentre() != m_pCentre || !(pEvent->GetEv() & ev))
			return -1;

		pEvent->PushCurEv(ev);

		activeEvents.push_back(pEvent.get());

		return 0;
	}


	void Poller::Clear()
	{
		for (auto ev : m_events)
		{
			ev.second->CancelEvent();
		}
	}


	Poller* Poller::AdapterNetDrive(EventCentre* pCentre)
	{
#ifdef _WIN32

#ifdef IOCP_ENABLE
		return new IOCP(pCentre);
#endif // IOCP_ENABLE

#else

#ifdef EPOLL_ENABLE
		return new Epoll(pCentre);
#endif // EPOLL_ENABLE

#endif	//_WIN32
		return new Select(pCentre);
	}
}
