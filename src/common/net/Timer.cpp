#include "Timer.h"
#include <time.h>


namespace chaos
{

	timer_id Timer::s_maxIDSize = Timer::INIT_ID_SIZE;
	char* Timer::s_ids = new char[INIT_ID_SIZE];
	uint32 Timer::s_curTimers = 0;

	Timer::Timer() :
		m_lastRunTime(0)
	{
		++s_curTimers;
	}


	Timer::~Timer()
	{
		--s_curTimers;
		if (s_ids && 0 >= s_curTimers)
			delete [] s_ids;
	}


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


	uint32 Timer::AddTimer(TimerEvent* pTimerEv)
	{
		if (!pTimerEv)
			return 0;

		const EventKey& key = pTimerEv->GetEvKey();

		int id = key.timerId;
		if (0 == id)
			return 0;

		if (0 != m_timers.Push(pTimerEv))
		{
			s_ids[id] = 0;
			return 0;
		}

		return id;

	}


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