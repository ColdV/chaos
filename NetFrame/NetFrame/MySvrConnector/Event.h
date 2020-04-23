#pragma once
#include "NetDrive.h"
//struct Event;

//typedef void (*EventCallback)(Event* pEvent, void* userData);

#include "../common/stdafx.h"
#include <map>
#include <set>

enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
	EV_SIGNAL = 1 << 4,
};

//struct TimerEvent
//{
//	unsigned int timerID;
//	int timeOut;
//	//TimerCallback cb;
//};
//
//
//struct SocketEvent		//need scoketEvMgr
//{
//	unsigned int fd;
//	int size;
//	int totalSize;
//	char* buffer;
//};
//
//
//struct Event
//{
//	int evID;
//	int ev;
//
//	union
//	{
//		TimerEvent		evTimer;
//		SocketEvent		evSocket;
//	} Ev;
//
//	EventCallback	evCb;
//	bool isLoop;
//	void* userData;
//};
//
//
//class EventNew
//{
//public:
//	virtual ~EventNew() = 0;
//
//protected:
//	EventNew() {};
//
//	EventNew* NewEvent(short ev, EventCallback evCb, bool isLoop) {};
//
//protected:
//	unsigned int m_id;
//	short m_ev;
//	EventCallback m_evCb;
//	bool m_isLoop;
//
//	static unsigned int m_increment;
//	static std::set<unsigned int> m_readyId;
//};


namespace NetFrame
{
	class EventHandler;
	class Event;
	class Socket;

	typedef std::map<Event*, EventHandler*> EventMap;

	//抽象事件(资源类)
	class Event
	{
	public:
		Event() {}
		virtual ~Event() = 0;

		void SetID(uint32 id) { m_id = id; }
		uint32 GetID() { return m_id; }

		void SetLoop(bool isLoop) { m_isLoop = isLoop; }
		bool IsLoop() { return m_isLoop; }

		void SetEv(uint32 ev) { m_ev = ev; }
		uint32 GetEv() { return m_ev; }

		void SetHandler(EventHandler* pHandler) { m_pHandler = pHandler; }

	private:
		uint32 m_id;
		bool m_isLoop;
		uint32 m_ev;
		EventHandler* m_pHandler;
	};


	//事件的注册、销毁、分发
	class EventCentre
	{
	public:
		EventCentre() {}
		~EventCentre() {}

		int Init();

		void EventLoop();

		int RegisterEvent(Event* ev, EventHandler* pHandler, bool isReady);

		int CancelEvent(Event* ev);

		int DispatchEvent();

		int NetEventDispatch();

		int SignalDispatch();

		int TimerDispatch();

		int ProcessReadyEvent();

	private:
		EventMap m_netEvs;			//IOMasterEvent->AllIOEvent

		NetDrive* m_pNetDrive;

		EventMap m_timerEvs;

		EventMap m_signalEvs;

		EventMap m_readyEv;
	};



	//抽象事件处理器
	class EventHandler
	{
	public:
		EventHandler() {}
		virtual ~EventHandler() = 0;

		virtual void Handle(Event* pEv) = 0;
	};


	//网络事件调度器
	class NetEventDispatcher
	{
	public:
		NetEventDispatcher();
		virtual ~NetEventDispatcher() = 0;

		virtual void Init() = 0;
	};


	class NetEvent : public Event
	{
	public:
		NetEvent(Socket* pSocket) { pSocket = m_pSocket; }
		~NetEvent()
		{
			if (m_pSocket)
				delete m_pSocket;
		}

	private:
		Socket* m_pSocket;
	};


	class NetEventHandler : public EventHandler
	{
	public:
		NetEventHandler();
		~NetEventHandler();

		virtual void Handle(Event* pEv);
	};

}	//namespace NetFrame