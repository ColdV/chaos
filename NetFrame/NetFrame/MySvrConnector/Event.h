#pragma once

struct Event;

typedef void (*EventCallback)(Event* pEvent, void* userData);

#include "../common/stdafx.h"
#include <map>
#include <set>

enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
};

struct TimerEvent
{
	unsigned int timerID;
	int timeOut;
	//TimerCallback cb;
};


struct SocketEvent		//need scoketEvMgr
{
	unsigned int fd;
	int size;
	int totalSize;
	char* buffer;
};


struct Event
{
	int evID;
	int ev;

	union
	{
		TimerEvent		evTimer;
		SocketEvent		evSocket;
	} Ev;

	EventCallback	evCb;
	bool isLoop;
	void* userData;
};


class EventNew
{
public:
	virtual ~EventNew() = 0;

protected:
	EventNew() {};

	EventNew* NewEvent(short ev, EventCallback evCb, bool isLoop) {};

protected:
	unsigned int m_id;
	short m_ev;
	EventCallback m_evCb;
	bool m_isLoop;

	static unsigned int m_increment;
	static std::set<unsigned int> m_readyId;
};


namespace EventFrame
{
	class EventHandler;

	//抽象事件(资源类)
	class Event
	{
	public:
		Event() {}
		virtual ~Event() = 0;

		void SetID(uint32 id) { m_id = id; }
		uint32 GetID() { return m_id; }

	private:
		uint32 m_id;
		//EventHandler* m_handler;
	};


	////事件启动器
	//class EventLauncher
	//{
	//public:
	//	EventLauncher() {}
	//	virtual ~EventLauncher() = 0;

	//	virtual void Run() = 0;
	//};


	//事件的注册、销毁、分发
	class EventCentre
	{
	public:
		EventCentre() {}
		~EventCentre() {}

		int Init();

		void Run();

		int RegisterEvent(Event* ev, EventHandler* pHandler);

		int CancelEvent(Event* ev);

		int DispatchEvent();

	private:
		std::map<Event*, EventHandler*> m_events;
		std::map<Event*, EventHandler*> m_readyEv;
	};



	//抽象事件处理器
	class EventHandler
	{
	public:
		EventHandler() {}
		virtual ~EventHandler() {}

		virtual void Handle(Event* pEv) = 0;
	};


}	//namespace EventFrame