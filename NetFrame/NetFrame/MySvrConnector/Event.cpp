#include "Event.h"

namespace EventFrame
{
	void EventCentre::Run()
	{
		while (true)
		{
			DispatchEvent();
		}
	}


	int EventCentre::DispatchEvent()
	{
		for (auto it = m_readyEv.begin(); it != m_readyEv.end(); ++it)
		{
			if (!it->first)
			{
				it = m_readyEv.erase(it);
				break;
			}

			if(it->second && it->first)
				it->second->Handle(it->first);
		}

		return 0;
	}


	int EventCentre::RegisterEvent(Event* ev, EventHandler* pHandler)
	{
		m_events.insert(std::make_pair(ev, pHandler));
		return 0;
	}

	int EventCentre::CancelEvent(Event* ev)
	{
		auto it = m_events.find(ev);
		if (it == m_events.end())
			return 0;

		if (it->first)
			delete it->first;

		if (it->second)
			delete it->second;

		m_events.erase(it);

		return 0;
	}
}