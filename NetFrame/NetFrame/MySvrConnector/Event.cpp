#include "Event.h"

namespace EventFrame
{
	void EventCentre::EventLoop()
	{
		while (true)
		{
			DispatchEvent();
			
			//timer events dispatch -- to ready

			//signal events dispatch -- to ready

			//io events dispatch	-- to ready
		}
	}


	int EventCentre::DispatchEvent()
	{
		//for (auto it = m_readyEv.begin(); it != m_readyEv.end(); ++it)
		//{
		//	if (!it->first)
		//	{
		//		it = m_readyEv.erase(it);
		//		break;
		//	}

		//	if(it->second && it->first)
		//		it->second->Handle(it->first);

		//	if (!it->first->IsLoop())
		//		CancelEvent(it->first);
		//}

		TimerDispatch();

		IOEventDispatch();

		SignalDispatch();

		return 0;
	}


	int EventCentre::ProcessReadyEvent()
	{
		for (auto it = m_readyEv.begin(); it != m_readyEv.end(); ++it)
		{
			if (!it->first)
			{
				continue;
			}

			if (it->second && it->first)
			{
				it->second->Handle(it->first);
				continue;
			}

			/*if (!it->first->IsLoop())
				CancelEvent(it->first);*/
		}

		m_readyEv.clear();

		return 0;
	}

	int EventCentre::RegisterEvent(Event* ev, EventHandler* pHandler, bool isReady)
	{
		//m_events.insert(std::make_pair(ev, pHandler));
		if (!ev || !pHandler)
			return -1;

		uint32 iEv = ev->GetEv();

		if (iEv & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
			m_netEvs.insert(std::make_pair(ev, pHandler));
		else if (iEv & EV_TIMEOUT)
			m_timerEvs.insert(std::make_pair(ev, pHandler));
		
		return 0;
	}

	int EventCentre::CancelEvent(Event* ev)
	{
		/*auto it = m_events.find(ev);
		if (it == m_events.end())
			return 0;

		if (it->first)
			delete it->first;

		if (it->second)
			delete it->second;

		m_readyEv.erase(it->first);
		m_events.erase(it);*/

		return 0;
	}
}