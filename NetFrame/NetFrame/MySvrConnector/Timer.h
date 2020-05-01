#pragma once

#if 0

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


struct TimerCmp
{
	bool operator()(Event& l, Event& r) const
	{
		return l.Ev.evTimer.timeOut < r.Ev.evTimer.timeOut;
	}

};

class Timer	//:public MinHeap<TimerEvent, TimerCmp>  每次调整位置后 设置TimerEvent中的POS字段
{
public:
	Timer();
	~Timer();

	void DispatchTimer();

	int AddTimer(Event* ev, int timeOut, EventCallback cb);

	int AddTimer(Event* ev, unsigned int hour, unsigned int min, unsigned int sec, EventCallback cb);

	//删除:将timer放入dellist删除列表中 每次执行定时器时 检测当前定时器如果在删除列表中 则删除
	int DelTimer(Event* ev);

	int DelTimer(unsigned int timerID);

	//void StartTimer();
	
	//void StopTimer();

	Event* PopTimer() { return TopTimer(); }

	Event* TopTimer() { return m_timers.Top(); }

	int GetSize() { return m_timers.GetSize(); }

	int GetTotalSize() { return m_timers.GetTotalSize(); }

private:
	MinHeap<Event, TimerCmp> m_timers;
	std::set<unsigned int>	m_timerIDs;
	std::set<unsigned int>	m_delList;
	std::set<unsigned int>	m_deled;
	time_t	m_lastRunTime;
};

#endif 