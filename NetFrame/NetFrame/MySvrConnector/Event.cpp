#include "Event.h"
#include "Timer.h"
#include <stdio.h>

namespace NetFrame
{
	//int Event::AddNewEvent(Event* pNewEv)
	//{
	//	if (!pNewEv || !m_pCenter)
	//		return -1;

	//	return m_pCenter->RegisterEvent(pNewEv, NULL);
	//}
}


namespace NetFrame
{
	EventCentre::EventCentre() :m_pNetDrive(0)
	{}


	EventCentre::~EventCentre()
	{
		if (m_pNetDrive)
			delete m_pNetDrive;

		if (!m_pTimer)
			delete m_pTimer;
	}


	int EventCentre::Init()
	{
		m_pNetDrive = NetDrive::AdapterNetDrive();
		if (!m_pNetDrive)
			return -1;

		m_pNetDrive->Init();

		m_pTimer = new Timer();
		if (!m_pTimer)
			return -1;

		return 0;
	}


	void EventCentre::EventLoop()
	{
		while (true)
		{
			if (0 != DispatchEvent())
				break;
		}
	}


	int EventCentre::DispatchEvent()
	{
		int ret = 0;

		if (0 != (ret = TimerDispatch()))
			return ret;

		if (0 != (ret = NetEventDispatch()))
			return ret;

		if (0 != (ret = SignalDispatch()))
			return ret;

		if(0 != (ret = ProcessActiveEvent()))
			return ret;

		return ret;
	}


	int EventCentre::ProcessActiveEvent()
	{
		for (auto it = m_activeEvs.begin(); it != m_activeEvs.end(); ++it)
		{
			if (!*it)
				continue;

			(*it)->Handle();
		}

		m_activeEvs.clear();

		return 0;
	}

	int EventCentre::RegisterEvent(Event* ev, EventHandler* pHandler)
	{
		//m_events.insert(std::make_pair(ev, pHandler));
		if (!ev)//|| !pHandler)
			return -1;

		uint32 iEv = ev->GetEv();

		//ev->SetHandler(pHandler);

		const EventKey* pEvKey = ev->GetEvKey();

		if (pEvKey)
		{
			if (iEv & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT)
			   && !(ev & ~(EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT)))
			{
				m_netEvs.insert(std::make_pair(pEvKey->fd, ev));
				m_pNetDrive->AddFd(pEvKey->fd, iEv);
			}
			else if (iEv & EV_TIMEOUT && !(ev & ~EV_TIMEOUT))
			{
				m_timerEvs.insert(std::make_pair(pEvKey->timerId, ev));
				m_pTimer->AddTimer((TimerEvent*)ev);
			}
			else if (iEv & EV_SIGNAL && !(ev & ~EV_SIGNAL))
				m_signalEvs.insert(std::make_pair(pEvKey->signal, ev));
			else
				return -1;
		}

		return 0;
	}

	int EventCentre::CancelEvent(Event* ev)
	{
		if (!ev)
			return -1;

		uint32 iEv = ev->GetEv();

		const EventKey* pEvKey = ev->GetEvKey();

		if (pEvKey)
		{
			if (iEv & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
				m_netEvs.erase(pEvKey->fd);
			else if (iEv & EV_TIMEOUT)
				m_timerEvs.erase(pEvKey->timerId);
			else if (iEv & EV_SIGNAL)
				m_signalEvs.erase(pEvKey->signal);
			else
				return -1;
		}

		return 0;
	}


	int EventCentre::NetEventDispatch()
	{
		if (!m_pNetDrive)
			return -1;

		int ret = 0;
		if (0 != (ret = m_pNetDrive->Launch()))
			return ret;

		const std::list<FdEvent>& evs = m_pNetDrive->GetActives();

		for (auto it = evs.begin(); it != evs.end(); ++it)
		{
			auto evIt = m_netEvs.find(it->fd);
			if (evIt == m_netEvs.end())
				continue;

			m_activeEvs.push_back(evIt->second);
		}

		m_pNetDrive->ResetActives();

		return ret;
	}


	int EventCentre::SignalDispatch()
	{
		return 0;
	}


	int EventCentre::TimerDispatch()
	{
		if (!m_pTimer)
			return -1;

		m_pTimer->DispatchTimer();

		return 0;
	}

}


namespace NetFrame
{

	void NetEvent::Handle()
	{
		uint32 ev = GetCurEv();

		if (!m_pSocket)
			return;

		if (ev & EV_IOREAD)
		{
			uint32 fdType = m_pSocket->GetFdType();

			if (fdType == SKT_LISTEN)
			{
				HandleListen();
			}
			else if (fdType == SKT_CONNING)
			{
				HandleRead();
			}
		}

		if (ev & EV_IOWRITE)
		{
			HandleWrite();
		}
	}


	int NetEvent::HandleListen()
	{
		if (!m_pSocket)
			return -1;

		Socket* pNewSock = m_pSocket->Accept2();
		if (!pNewSock)
			return -1;

		EventKey* pKey = new EventKey();
		if (!pKey)
			return -1;

		pKey->fd = pNewSock->GetFd();

		NetEventHandler* pHandler = new NetEventHandler();
		if (!pHandler)
			return -1;

		EventCentre* pCentre = GetCentre();
		if (!pCentre)
			return -1;

		NetEvent* pNewEv = new NetEvent(pCentre, pNewSock, EV_IOREAD | EV_IOWRITE, pHandler, pKey);
		if (!pNewEv)
			return -1;

		/*if (0 != AddNewEvent(pNewEv))
			return -1;*/
		pCentre->RegisterEvent(pNewEv, NULL);

		return 0;
	}


	int NetEvent::HandleRead()
	{
		return m_pSocket && m_pRBuffer ?
			m_pRBuffer->ReadFd(m_pSocket) :
			-1;
	}


	int NetEvent::HandleWrite()
	{
		return m_pSocket && m_pWBuffer ?
			m_pWBuffer->WriteFd(m_pSocket) :
			-1;
	}
}


namespace NetFrame
{ 

	void NetEventHandler::Handle(Event* pEv)
	{
		NetEvent* pNetEv = (NetEvent*)pEv;

		uint32 ev = pNetEv->GetCurEv();

		Socket* pSocket = pNetEv->GetSocket();
		if (!pSocket)
			return;

		if (ev & EV_IOREAD)
		{
			uint32 fdType = pSocket->GetFdType();

			if (fdType == SKT_LISTEN)
			{
				HandleListen(pNetEv);
			}
			else if (fdType == SKT_CONNING)
			{
				HandleRead(pNetEv);
			}
		}
		
		if (ev & EV_IOWRITE)
		{
			HandleWrite(pNetEv);
		}
	}


	int NetEventHandler::HandleListen(NetEvent* pNetEv)
	{
		if (!pNetEv)
			return -1;

		Socket* pSocket = pNetEv->GetSocket();
		if (!pSocket)
			return -1;

		Socket* pNewSock = pSocket->Accept2();
		if (!pNewSock)
			return -1;

		EventKey* pKey = new EventKey();
		if (!pKey)
			return -1;

		pKey->fd = pNewSock->GetFd();
	
		NetEventHandler* pHandler = new NetEventHandler();
		if (!pHandler)
			return -1;

		/*NetEvent* pNewEv = new NetEvent(pNewSock, EV_IOREAD | EV_IOWRITE, pHandler, pKey);
		if (!pNewEv)
			return -1;*/

		return 0;
	}


	int NetEventHandler::HandleRead(NetEvent* pNetEv)
	{
		if (!pNetEv)
			return -1;

		Socket* pSocket = pNetEv->GetSocket();
		if (!pSocket)
			return -1;

		return 0;
	}


	int NetEventHandler::HandleWrite(NetEvent* pNetEv)
	{
		return 0;
	}
}


namespace NetFrame
{
	void TimerEvent::Handle()
	{
		printf("test!\n");
	}
}
