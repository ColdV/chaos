#include "IOEventMgr.h"

IOEventMgr::IOEventMgr()
{
}

IOEventMgr::~IOEventMgr()
{
}


int IOEventMgr::AddIOEevent(const Event& ev)
{
	m_ioEvents.insert(std::make_pair(ev.Ev.evSocket.fd, ev));
	return 0;
}


void IOEventMgr::DelIOEvent(const Event& ev)
{
	auto end_it = m_ioEvents.upper_bound(ev.Ev.evSocket.fd);

	for (auto it = m_ioEvents.lower_bound(ev.Ev.evSocket.fd); it != end_it; ++it)
	{
		if (it->second.ev == ev.ev)
		{
			it = m_ioEvents.erase(it);
			break;
		}
	}
}
