/************C++ Source File****************
#
#	Filename: NetDrive.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:02
#	Last Modified: 2018-08-11 17:40:02
*******************************************/


#include "NetDrive.h"
#include "Select.h"
#include "Epoll.h"
#include "IOCP.h"
#include "Event.h"

namespace NetFrame
{

	NetDrive::NetDrive(EventCentre* pCentre):
		m_pCentre(pCentre)
	{
		//m_fds.clear();
		//m_activeFd.clear();
	}


	NetDrive::~NetDrive()
	{
		if (m_pCentre)
			delete m_pCentre;
		printf("delete IO!\n");
	}


	int NetDrive::AddEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		const EventKey* pKey = pEvent->GetEvKey();
		if (!pKey)
			return -1;

		m_events.insert(std::make_pair(pKey->fd, pEvent));

		RegistFd(pKey->fd, pEvent->GetEv());

		return 0;
	}

	
	int NetDrive::DelEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		const EventKey* pKey = pEvent->GetEvKey();

		if (!pKey)
			return -1;

		return DelEvent(pKey->fd);
	}


	int NetDrive::DelEvent(socket_t fd)
	{
		CancelFd(fd);

		auto it = m_events.find(fd);

		if (it == m_events.end())
			return -1;

		delete it->second;
		m_events.erase(fd);


		return 0;
	}


	Event* NetDrive::GetEvent(socket_t fd)
	{
		auto it = m_events.find(fd);

		if (it == m_events.end())
			return NULL;

		return it->second;
	}


	int NetDrive::PushActiveEvent(socket_t fd, short ev)
	{
		auto it = m_events.find(fd);

		if (it == m_events.end())
			return -1;

		it->second->SetCurEv(it->second->GetCurEv() | ev);

		return PushActiveEvent(it->second);
	}


	int NetDrive::PushActiveEvent(Event* pEvent)
	{
		if (!pEvent || !m_pCentre)
			return -1;

		m_pCentre->PushActiveEv(pEvent);

		return 0;
	}


	NetDrive* NetDrive::AdapterNetDrive(EventCentre* pCentre)
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