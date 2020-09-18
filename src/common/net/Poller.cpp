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
		//if (m_pCentre)
		//	delete m_pCentre;
		//printf("delete IO!\n");
	}


	int Poller::AddEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		const EventKey& key = pEvent->GetEvKey();

		if (0 != RegistFd(key.fd, pEvent->GetEv()))
			return -1;

		m_events.insert(std::make_pair(key.fd, pEvent));

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
		
		//auto it = m_events.find(fd);
		//
		//if (it == m_events.end())
		//	return -1;

		//delete it->second;
		/*if (0 != m_events.erase(fd))
			return 0;

		return 0;*/

		m_events.erase(fd);
		return 0;//m_events.erase(fd) ? 0 : 1;
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

		//it->second->SetCurEv(it->second->GetCurEv() | ev);

		if (!m_pCentre)
			return -1;
		
		m_pCentre->PushActiveEv(it->second, ev);

		return 0;
		//return PushActiveEvent(it->second, ev);
	}


	//int Poller::PushActiveEvent(Event* pEvent)
	//{
	//	if (!pEvent || !m_pCentre)
	//		return -1;

	//	m_pCentre->PushActiveEv(pEvent);

	//	return 0;
	//}


	Poller* Poller::AdapterNetDrive(EventCentre* pCentre)
	{
#ifdef _WIN32
		//return &Select::Instance();
		//return new Select(pCentre);
		return new IOCP(pCentre);
#else
		/*return &Epoll:Instance();*/
		return new Epoll(pCentre);
#endif

		return 0;
	}
}
