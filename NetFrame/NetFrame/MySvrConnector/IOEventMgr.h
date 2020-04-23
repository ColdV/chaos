#pragma once

#include "Event.h"
#include <map>

#if 0
class IOEventMgr
{
public:
	IOEventMgr();
	~IOEventMgr();

	int AddIOEevent(const Event& ev);

	void DelIOEvent(const Event& ev);


private:
	std::multimap<unsigned int, Event> m_ioEvents;
};

#endif