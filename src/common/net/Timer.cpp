#include "Timer.h"
#include <time.h>


namespace chaos
{
	std::vector<byte> Timer::s_ids(INIT_ID_SIZE);

	std::atomic<uint32> Timer::s_curTimers(0);

	Mutex Timer::s_mutex;


	Timer::Timer(EventCentre* pCentre) :
		m_pCentre(pCentre),
		m_lastRunTime(0)
	{
	}


	Timer::~Timer()
	{
	}


	void Timer::Launch(EventList& activeEvents)
	{
		m_lastRunTime = GetCurrentMSec();

		while (!m_timers.Empty())
		{
			TimerSharedPtr pEvent(m_timers.Front().lock());

			if (!pEvent)
			{
				//这里不需要去调用DelTimer(TimerMap中是shared_ptr如果执行到这里,TimerMap中必然不存在该元素)
				m_timers.Pop();
				continue;
			}

			if (m_lastRunTime < pEvent->GetNextTime())
				break;

			if (pEvent->IsSuspend())
			{
				//暂停的定时器如果不是循环的就直接删除
				if (pEvent->IsLoop())
				{
					pEvent->SetNextTime();
					m_timers.Push(pEvent);
				}
				else
				{
					DelTimer(pEvent.get());
				}

				m_timers.Pop();
				continue;
			}

			activeEvents.push_back(pEvent.get());

			//循环定时任务
			if (pEvent->IsLoop())
			{
				pEvent->SetNextTime();
				m_timers.Push(pEvent);
			}

			m_timers.Pop();

		}
	}
		

	uint32 Timer::AddTimer(const TimerSharedPtr& pTimerEv)
	{
		if (!pTimerEv)
			return -1;

		const EventKey& key = pTimerEv->GetEvKey();

		int id = key.timerId;

		if (m_timerMap.find(id) != m_timerMap.end())
			return -1;

		m_timerMap.insert(std::make_pair(id, pTimerEv));
		
		m_timers.Push(pTimerEv);

		++s_curTimers;

		if (m_pCentre)
			m_pCentre->WakeUp();

		return 0;

	}


	uint32 Timer::DelTimer(TimerEvent* pTimerEv)
	{
		if (!pTimerEv)
			return -1;

		m_timerMap.erase(pTimerEv->GetEvKey().timerId);

		--s_curTimers;

		return 0;
	}


	int Timer::GetNextTimeout()
	{
		if (!m_timers.Empty())
		{
			TimerSharedPtr pEvent(m_timers.Front().lock());
			if (!pEvent)
				return 0;

			TIME_T timeout = pEvent->GetNextTime() - GetCurrentMSec();
			
			return (int)(timeout > 0 ? timeout : 0);
		}

		return -1;
	}


	void Timer::Clear()
	{
		while (!m_timers.Empty())
		{
			TimerSharedPtr pEvent(m_timers.Front().lock());
			m_timers.Pop();

			if (!pEvent)
				continue;

			pEvent->CancelEvent();
		}
	}


	timer_id Timer::CreateTimerID()
	{
		MutexGuard lock(s_mutex);

		if (s_curTimers >= s_ids.size() * BYTE2BIT)
			s_ids.emplace_back(0);

		uint32 max = s_ids.size();
		uint32 i = 0;

		while(i < max)
		{
			byte& idBytes = s_ids[i++];

			byte bit = 0;
			while (bit < BYTE2BIT)
			{
				byte bitPos = bit++;
				if (!GetBit(idBytes, bitPos))
				{
					SetBit(idBytes, bitPos);
					return (i - 1) * BYTE2BIT + bit;
				}
			}
		}

		return 0;
	}


	void Timer::ReleaseTimerID(timer_id id)
	{
		if (0 == id)
			return;

		int pos = id / BYTE2BIT;
		int bit = id % BYTE2BIT;

		MutexGuard lock(s_mutex);

		if (pos >= s_ids.size())
			return;

		if (0 == bit)
			ClrBit(s_ids[pos], BYTE2BIT - 1);
		else
			ClrBit(s_ids[pos], bit - 1);
	}

}