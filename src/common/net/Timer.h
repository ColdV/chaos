#pragma once

#include <set>
#include <unordered_map>
#include "template/MinHeap.h"
#include "Event.h"


namespace chaos
{ 

	struct TimerCmp
	{
		bool operator()(const std::weak_ptr<TimerEvent>& weakl, const std::weak_ptr<TimerEvent>& weakr) const
		{
			std::shared_ptr<TimerEvent> l(weakl.lock());
			std::shared_ptr<TimerEvent> r(weakr.lock());
			if (!l || !r)
				return false;

			return l->GetNextTime() < r->GetNextTime();
		}

	};

	class Timer
	{
	public:
		static const int INIT_ID_SIZE = 128;
		static const int BYTE2BIT = 8;

		friend EventCentre;
		friend TimerEvent;

		typedef std::unordered_map<int, std::shared_ptr<TimerEvent>> TimerMap;
		typedef std::shared_ptr<TimerEvent> TimerSharedPtr;
		typedef std::weak_ptr<TimerEvent> TimerWeakPtr;

		Timer(EventCentre* pCentre);
		~Timer();

		void Launch(EventList& activeEvents);

		//添加定时器,返回定时器ID
		uint32 AddTimer(const EventSharedPtr& pEvent) { return AddTimer(std::static_pointer_cast<TimerEvent>(pEvent)); }

		uint32 AddTimer(const TimerSharedPtr& pTimerEv);

		uint32 DelTimer(TimerEvent* pTimerEv);

		//获取下一次超时时间
		//-1:当前没有定时器, 0:当前已有就绪的定时器, >0:下个定时器超时时间
		int GetNextTimeout();

		int Size() { return m_timers.Size(); }

		void Clear();

		//分配一个定时器ID, 返回0表示无ID可用
		static timer_id CreateTimerID();

		static void ReleaseTimerID(timer_id id);

	private:
		TimerMap& GetAllTimer() { return m_timerMap; }

	private:
		MinHeap<TimerWeakPtr, TimerCmp> m_timers;

		EventCentre* m_pCentre;

		TimerMap m_timerMap;

		TIME_T	m_lastRunTime;

	private:
		static std::vector<byte> s_ids;

		static std::atomic<uint32> s_curTimers;			//定时器个数

		static Mutex s_mutex;
	};

}
