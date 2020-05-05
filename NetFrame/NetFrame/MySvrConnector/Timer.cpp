#include "Timer.h"
#include <time.h>
//#include <vector>
//
//std::vector<int> v;


namespace NetFrame
{

	Timer::Timer() :
		m_maxIDSize(INIT_ID_SIZE),
		m_lastRunTime(0)
	{
		m_ids = new char[INIT_ID_SIZE];
	}


	Timer::~Timer()
	{
		if (m_ids)
			delete [] m_ids;
	}


	//void Timer::DispatchTimer()
	//{
	//	time_t curTime = time(NULL);

	//	Event* ev = TopTimer();

	//	while (ev && curTime - m_lastRunTime >= ev->Ev.evTimer.timeOut)
	//	{
	//		if (!m_delList.empty()
	//			&& m_delList.find(ev->Ev.evTimer.timerID) != m_delList.end())
	//		{
	//			PopTimer();
	//			ev = TopTimer();
	//			m_delList.erase(ev->Ev.evTimer.timerID);
	//			continue;
	//		}

	//		ev = PopTimer();
	//		ev->evCb(ev, ev->userData);

	//		if (ev->isLoop)
	//			AddTimer(ev, ev->Ev.evTimer.timeOut, ev->evCb);

	//		ev = TopTimer();
	//	}

	//	m_lastRunTime = time(NULL);
	//}


	void Timer::DispatchTimer()
	{
		time_t curTime = time(NULL);

		TimerEvent* ev = TopTimer();

		while (ev && curTime - m_lastRunTime >= ev->GetTimeOut())
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


	//int Timer::AddTimer(Event* ev, int timeOut, EventCallback cb)
	//{
	//	if (!ev)
	//		return -1;
	//
	//	ev->evCb = cb;
	//	ev->Ev.evTimer.timeOut = timeOut;
	//
	//	unsigned int& timerID = ev->Ev.evTimer.timerID;
	//		
	//	if (m_deled.empty())
	//	{
	//		timerID = m_timers.GetSize() + 1;
	//	}
	//	else
	//	{
	//		timerID = *(m_deled.begin());
	//		m_deled.erase(m_deled.begin());
	//	}
	//
	//	if (0 != m_timers.Push(*ev))
	//		return -1;
	//
	//	m_timerIDs;
	//
	//	return timerID;
	//}
	//
	//
	//
	//int Timer::AddTimer(Event* ev, unsigned int hour, unsigned int min, unsigned int sec, EventCallback cb)
	//{
	//	tm t;
	//	time_t now = time(NULL);
	//
	//#ifdef _WIN32
	//
	//	if (0 != localtime_s(&t, &now))
	//		return -1;
	//
	//#else
	//	if (!localtime_r(&now, &t))
	//		return -1;
	//
	//#endif // _WIN32
	//
	//	t.tm_hour = hour;
	//	t.tm_min = min;
	//	t.tm_sec = sec;
	//
	//	time_t timeOut = mktime(&t) - time(NULL);
	//
	//	return AddTimer(ev, timeOut, cb);
	//}


	uint32 Timer::AddTimer(TimerEvent* pTimerEv)
	{
		if (!pTimerEv)
			return 0;

		int id = AllocaTimerID();
		if (0 == id)
			return 0;

		if (0 != m_timers.Push(pTimerEv))
		{
			m_ids[id] = 0;
			return 0;
		}

		return id;

	}


	//int Timer::DelTimer(Event* ev)
	//{
	//	if (!ev)
	//		return -1;

	//	return DelTimer(ev->Ev.evTimer.timerID);
	//}


	//int Timer::DelTimer(unsigned int timerID)
	//{
	//	if (m_timerIDs.find(timerID) == m_timerIDs.end())
	//		return -1;

	//	if (!m_delList.insert(timerID).second)
	//		return -1;

	//	m_timerIDs.erase(timerID);
	//	m_deled.insert(timerID);

	//	return timerID;
	//}


	uint32 Timer::DelTimer(TimerEvent* pTimerEv)
	{
		for (TimerEvent* const* p = m_timers.Begin(); p != m_timers.End(); ++p)
		{
			if (*p == pTimerEv)
			{
				m_timers.Erase(p);
				return 0;
			}
		}

		return 0;
	}


	uint32 Timer::AllocaTimerID()
	{
		if (!m_ids)
			return 0;

		uint32 i = 0;
		while(i < m_maxIDSize)
		{
			if (m_ids[i] != 1)
			{
				m_ids[i] = 1;
				return i + 1;
			}

			++i;
		}

		//当前ID 已用完 扩展ID
		if (i == m_maxIDSize && 0 == ExpandID())
			return i;

		return 0;
	}


	int Timer::ExpandID()
	{
		if (!m_ids || m_maxIDSize >= 0xFFFFFFFF)
			return -1;

		uint32 size = 0xFFFFFFFF / 2 > m_maxIDSize ? 0xFFFFFFFF : m_maxIDSize * 2;

		char* pNewIDs = new char[size] {0};
		if (!pNewIDs)
			return -1;
		
		memcpy(pNewIDs, m_ids, m_maxIDSize);
		delete[] m_ids;
		m_ids = pNewIDs;
		m_maxIDSize = size;

		return 0;
	}

}