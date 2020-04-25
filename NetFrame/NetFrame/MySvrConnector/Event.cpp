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

		ev->SetHandler(pHandler);

		const EventKey& evKey = ev->GetEvKey();

		if (iEv & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
			m_netEvs.insert(std::make_pair(evKey.netEvKey, ev));
		else if (iEv & EV_TIMEOUT)
			m_timerEvs.insert(std::make_pair(evKey.timerEvKey, ev));
		else if (iEv & EV_SIGNAL)
			m_signalEvs.insert(std::make_pair(evKey.signalEvKey, ev));
		else
			return -1;

		return 0;
	}

	int EventCentre::CancelEvent(Event* ev)
	{
		if (!ev)
			return -1;

		uint32 iEv = ev->GetEv();

		EventKey evKey = ev->GetEvKey();

		if (iEv & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
			m_netEvs.erase(evKey.netEvKey);
		else if (iEv & EV_TIMEOUT)
			m_timerEvs.erase(evKey.timerEvKey);
		else if (iEv & EV_SIGNAL)
			m_signalEvs.erase(evKey.signalEvKey);
		else
			return -1;

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
	void NetEvent::Handle()
	{

	}
}