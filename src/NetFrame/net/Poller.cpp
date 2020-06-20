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
		//m_fds.clear();
		//m_activeFd.clear();
	}


	Poller::~Poller()
	{
		if (m_pCentre)
			delete m_pCentre;
		printf("delete IO!\n");
	}


	int Poller::AddEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		const EventKey& key = pEvent->GetEvKey();

		m_events.insert(std::make_pair(key.fd, pEvent));

		RegistFd(key.fd, pEvent->GetEv());

		return 0;
	}

	
	int Poller::DelEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

 		const EventKey& key = pEvent->GetEvKey();

		return DelEvent(key.fd);
	}


	int Poller::DelEvent(socket_t fd)
	{
		CancelFd(fd);
		
		auto it = m_events.find(fd);
		
		if (it == m_events.end())
			return -1;

		delete it->second;
		m_events.erase(fd);


		return 0;
	}


	Event* Poller::GetEvent(socket_t fd)
	{
		auto it = m_events.find(fd);

		if (it == m_events.end())
			return NULL;

		return it->second;
	}


	int Poller::PushActiveEvent(socket_t fd, short ev)
	{
		auto it = m_events.find(fd);

		if (it == m_events.end())
			return -1;

		it->second->SetCurEv(it->second->GetCurEv() | ev);

		return PushActiveEvent(it->second);
	}


	int Poller::PushActiveEvent(Event* pEvent)
	{
		if (!pEvent || !m_pCentre)
			return -1;

		m_pCentre->PushActiveEv(pEvent);

		return 0;
	}


	Poller* Poller::AdapterNetDrive(EventCentre* pCentre)
	{
#ifdef _WIN32
		//return &Select::Instance();
		return new Select(pCentre);
#else
		/*return &Epoll:Instance();*/
		return new Epoll(pCentre);
#endif

		return 0;
	}
}
