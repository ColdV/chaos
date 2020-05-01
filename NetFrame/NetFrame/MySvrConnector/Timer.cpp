#include "Timer.h"
#include <time.h>
//#include <vector>
//
//std::vector<int> v;

#if 0

Timer::Timer()
{
}


Timer::~Timer()
{
}


void Timer::DispatchTimer()
{
	time_t curTime = time(NULL);

	Event* ev = TopTimer();

	while (ev && curTime - m_lastRunTime >= ev->Ev.evTimer.timeOut)
	{
		if (!m_delList.empty()
			&& m_delList.find(ev->Ev.evTimer.timerID) != m_delList.end())
		{
			PopTimer();
			ev = TopTimer();
			m_delList.erase(ev->Ev.evTimer.timerID);
			continue;
		}

		ev = PopTimer();
		ev->evCb(ev, ev->userData);
		
		if (ev->isLoop)
			AddTimer(ev, ev->Ev.evTimer.timeOut, ev->evCb);

		ev = TopTimer();
	}

	m_lastRunTime = time(NULL);
}


int Timer::AddTimer(Event* ev, int timeOut, EventCallback cb)
{
	if (!ev)
		return -1;

	ev->evCb = cb;
	ev->Ev.evTimer.timeOut = timeOut;

	unsigned int& timerID = ev->Ev.evTimer.timerID;
		
	if (m_deled.empty())
	{
		timerID = m_timers.GetSize() + 1;
	}
	else
	{
		timerID = *(m_deled.begin());
		m_deled.erase(m_deled.begin());
	}

	if (0 != m_timers.Push(*ev))
		return -1;

	m_timerIDs;

	return timerID;
}



int Timer::AddTimer(Event* ev, unsigned int hour, unsigned int min, unsigned int sec, EventCallback cb)
{
	tm t;
	time_t now = time(NULL);

#ifdef _WIN32

	if (0 != localtime_s(&t, &now))
		return -1;

#else
	if (!localtime_r(&now, &t))
		return -1;

#endif // _WIN32

	t.tm_hour = hour;
	t.tm_min = min;
	t.tm_sec = sec;

	time_t timeOut = mktime(&t) - time(NULL);

	return AddTimer(ev, timeOut, cb);
}


int Timer::DelTimer(Event* ev)
{
	if (!ev)
		return -1;

	return DelTimer(ev->Ev.evTimer.timerID);
}


int Timer::DelTimer(unsigned int timerID)
{
	if (m_timerIDs.find(timerID) == m_timerIDs.end())
		return -1;

	if (!m_delList.insert(timerID).second)
		return -1;

	m_timerIDs.erase(timerID);
	m_deled.insert(timerID);

	return timerID;
}

#endif