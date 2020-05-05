#pragma once


#include"MinHeap.h"
#include <set>
#include "Event.h"

//typedef void (*TimerCallback)(TimerEvent* pEvent, void* userData);
//
//struct TimerEvent
//{
//	unsigned int timerID;
//	int timeOut;
//	TimerCallback cb;
//};

namespace NetFrame
{ 

	struct TimerCmp
	{
		bool operator()(TimerEvent* l, TimerEvent* r) const
		{
			return l->GetTimeOut() < r->GetTimeOut();
		}

	};

	class Timer	//:public MinHeap<TimerEvent, TimerCmp>  每次调整位置后 设置TimerEvent中的POS字段
	{
	public:
		enum 
		{
			INIT_ID_SIZE = 128,
		};

		Timer();
		~Timer();

		void DispatchTimer();

		//int AddTimer(Event* ev, int timeOut, EventCallback cb);

		//int AddTimer(Event* ev, unsigned int hour, unsigned int min, unsigned int sec, EventCallback cb);

		//添加定时器,返回定时器ID
		uint32 AddTimer(TimerEvent* pTimerEv);

		//删除:将timer放入dellist删除列表中 每次执行定时器时 检测当前定时器如果在删除列表中 则删除
		//int DelTimer(Event* ev);

		//int DelTimer(unsigned int timerID);

		uint32 DelTimer(TimerEvent* pTimerEv);

		//void StartTimer();
	
		//void StopTimer();

		//Event* PopTimer() { return TopTimer(); }

		//Event* TopTimer() { return m_timers.Top(); }

		int GetSize() { return m_timers.GetSize(); }

		int GetTotalSize() { return m_timers.GetTotalSize(); }

	private:
		//uint32 NewID() { if (m_maxID >= 0xFFFFFFFF)return 0; return ++m_maxID; }

		//TimerEvent* PopTimer() { return TopTimer(); }

		//TimerEvent* TopTimer() { return *m_timers.Front(); }

		//分配一个定时器ID, 返回0表示无ID可用
		uint32 AllocaTimerID();

		//扩展ID
		int ExpandID();

	private:
		MinHeap<TimerEvent*, TimerCmp> m_timers;
		//std::set<unsigned int>	m_timerIDs;
		std::set<unsigned int>	m_delList;
		std::set<unsigned int>	m_deled;
		char* m_ids;
		uint32 m_maxIDSize;
		time_t	m_lastRunTime;
	};

}
