#include "Event.h"



namespace NetFrame
{

	EventCentre::EventCentre() :m_pNetDrive(0)
	{}


	EventCentre::~EventCentre()
	{}


	int EventCentre::Init()
	{
		m_pNetDrive = NetDrive::AdapterNetDrive();
		m_pNetDrive->Init();

		return 0;
	}


	void EventCentre::EventLoop()
	{
		while (true)
		{
			DispatchEvent();
		}
	}


	int EventCentre::DispatchEvent()
	{
		TimerDispatch();

		NetEventDispatch();

		SignalDispatch();

		ProcessActiveEvent();

		return 0;
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
		if (!ev || !pHandler)
			return -1;

		uint32 iEv = ev->GetEv();

		//ev->SetHandler(pHandler);

		const EventKey* pEvKey = ev->GetEvKey();

		if (pEvKey)
		{
			if (iEv & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
				m_netEvs.insert(std::make_pair(pEvKey->fd, ev));
			else if (iEv & EV_TIMEOUT)
				m_timerEvs.insert(std::make_pair(pEvKey->timerId, ev));
			else if (iEv & EV_SIGNAL)
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

		m_pNetDrive->Launch();

		const std::list<FdEvent>& evs = m_pNetDrive->GetActives();

		for (auto it = evs.begin(); it != evs.end(); ++it)
		{
			auto evIt = m_netEvs.find(it->fd);
			if (evIt == m_netEvs.end())
				continue;

			m_activeEvs.push_back(evIt->second);
		}

		m_pNetDrive->ResetActives();

		return 0;
	}


	int EventCentre::SignalDispatch()
	{
		return 0;
	}


	int EventCentre::TimerDispatch()
	{
		return 0;
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
				HandleListen(pSocket);
			}
			else if (fdType == SKT_CONNING)
			{
				HandleRead(pSocket);
			}
		}
		
		if (ev & EV_IOWRITE)
		{
			HandleWrite(pSocket);
		}
	}


	int NetEventHandler::HandleListen(Socket* pSocket)
	{
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

		NetEvent* pNewEv = new NetEvent(pNewSock, EV_IOREAD | EV_IOWRITE, pHandler, pKey);
		if (!pNewEv)
			return -1;

		return 0;
	}


	int NetEventHandler::HandleRead(Socket* pSocket)
	{
		return 0;
	}


	int NetEventHandler::HandleWrite(Socket* pSocket)
	{
		return 0;
	}
}