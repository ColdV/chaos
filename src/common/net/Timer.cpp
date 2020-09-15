#include "Timer.h"
#include <time.h>


namespace chaos
{

	timer_id Timer::s_maxIDSize = Timer::INIT_ID_SIZE;

	char* Timer::s_ids = new char[INIT_ID_SIZE];

	uint32 Timer::s_curTimers = 0;


	//Timer& Timer::Instance()
	//{
	//	static Timer g_timer;
	//	return g_timer;
	//}


	Timer::Timer() :
		m_lastRunTime(0)
	{
	}


	Timer::~Timer()
	{
		if (s_ids && 0 >= s_curTimers)
			delete [] s_ids;
	}


	void Timer::DispatchTimer()
	{
		while (!m_timers.Empty())
		{
			TimerEvent* ev = m_timers.Front();
			if (!ev)
				continue;

			if (ev->IsCancel())
			{
				m_timers.Pop();
				DelTimer(ev);
				continue;
			}

			if (ev->IsSuspend())
			{
				//暂停的定时器如果不是循环的就直接删除
				if (ev->IsLoop())
				{
					ev->SetNextTime();
					AddTimer(ev);
				}
				else
				{
					m_timers.Pop();
					DelTimer(ev);
				}

				continue;
			}

			if (m_lastRunTime < ev->GetNextTime())
				break;

			EventCentre* pCentre = ev->GetCentre();
			if (!pCentre)
				continue;

			pCentre->PushActiveEv(ev);

			//循环定时任务
			if (ev->IsLoop())
			{
				ev->SetNextTime();
				AddTimer(ev);
			}

			m_timers.Pop();
		}

		m_lastRunTime = time(NULL);
	}


	uint32 Timer::AddTimer(TimerEvent* pTimerEv)
	{
		if (!pTimerEv)
			return -1;

		const EventKey& key = pTimerEv->GetEvKey();

		int id = key.timerId;
		if (0 >= id)
			return -1;

		if (0 > m_timers.Push(pTimerEv))
		{
			s_ids[id] = 0;
			return -1;
		}

		if (m_timerMap.find(id) == m_timerMap.end())
		{
			m_timerMap.insert(std::make_pair(id, pTimerEv));
			++s_curTimers;
		}

		return 0;

	}


	uint32 Timer::DelTimer(TimerEvent* pTimerEv)
	{
		/*for (TimerEvent* const* p = m_timers.Begin(); p != m_timers.End(); ++p)
		{
			if (*p == pTimerEv)
			{
				m_timers.Erase(p);
				--s_curTimers;
				break;
			}
		}*/

		if (!pTimerEv)
			return 0;

		m_timerMap.erase(pTimerEv->GetEvKey().timerId);

		--s_curTimers;

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