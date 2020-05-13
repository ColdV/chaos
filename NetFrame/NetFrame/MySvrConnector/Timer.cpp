#include "Timer.h"
#include <time.h>
//#include <vector>
//
//std::vector<int> v;


namespace NetFrame
{

	timer_id Timer::s_maxIDSize = Timer::INIT_ID_SIZE;
	char* Timer::s_ids = new char[INIT_ID_SIZE];

	Timer::Timer() :
		//s_maxIDSize(INIT_ID_SIZE),
		m_lastRunTime(0)
	{
		//s_ids = new char[INIT_ID_SIZE];
	}


	Timer::~Timer()
	{
		/*if (s_ids)
			delete [] s_ids;*/
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

		/*if (!pCentre)
			return;*/

		time_t curTime = time(NULL);

		//TimerEvent* ev = TopTimer();
		TimerEvent* ev = NULL;
		if (0 < m_timers.Size())
			ev = m_timers.Front();

		if (!ev)
			return;

		std::list<TimerEvent*> addEv;		//这里可以考虑优化

		while (ev && curTime - m_lastRunTime >= ev->GetTimeOut())
		{
			//if (!m_delList.empty()
			//	&& m_delList.find(ev->Ev.evTimer.timerID) != m_delList.end())
			//{
			//	PopTimer();
			//	ev = TopTimer();
			//	m_delList.erase(ev->Ev.evTimer.timerID);
			//	continue;
			//}

			/*ev = PopTimer();
			ev->evCb(ev, ev->userData);

			if (ev->isLoop)
				AddTimer(ev, ev->Ev.evTimer.timeOut, ev->evCb);

			ev = TopTimer();*/

			if (ev->IsLoop())
				/*AddTimer(ev);*/
				addEv.push_back(ev);

			/*pCentre->PushActiveEv(ev);*/

			if (ev->GetCentre())
				ev->GetCentre()->PushActiveEv(ev);

			m_timers.Pop();

			if (0 >= m_timers.Size())
				break;

			ev = m_timers.Front();
		}

		for (auto it = addEv.begin(); it != addEv.end(); ++it)
		{
			AddTimer(ev);
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

		const EventKey* pKey = pTimerEv->GetEvKey();
		if (!pKey)
			return 0;

		int id = pKey->timerId;
		if (0 == id)
			return 0;

		if (0 != m_timers.Push(pTimerEv))
		{
			s_ids[id] = 0;
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


	timer_id Timer::CreateTimerID()
	{
		if (!s_ids)
			return 0;

		uint32 i = 0;
		while(i < s_maxIDSize)
		{
			if (s_ids[i] != 1)
			{
				s_ids[i] = 1;
				return i + 1;
			}

			++i;
		}

		//当前ID 已用完 扩展ID
		if (i == s_maxIDSize && 0 == Timer::ExpandID())
			return i;

		return 0;
	}


	int Timer::ExpandID()
	{
		if (!s_ids || s_maxIDSize >= 0xFFFFFFFF)
			return -1;

		uint32 size = 0xFFFFFFFF / 2 > s_maxIDSize ? 0xFFFFFFFF : s_maxIDSize * 2;

		char* pNewIDs = new char[size] {0};
		if (!pNewIDs)
			return -1;
		
		memcpy(pNewIDs, s_ids, s_maxIDSize);
		delete[] s_ids;
		s_ids = pNewIDs;
		s_maxIDSize = size;

		return 0;
	}

}