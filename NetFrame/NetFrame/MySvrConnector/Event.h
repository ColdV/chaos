#pragma once


#include "../common/stdafx.h"
#include <map>
#include <set>
#include "NetDrive.h"

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

	typedef std::map<socket_t, Event*> NetEventMap;
	typedef std::map<int, Event*>	TimerEventMap;
	typedef std::map<int, Event*>	SignalEventMap;
	typedef std::list<Event*>	ActiveEventList;


	union EventKey
	{
		socket_t	netEvKey;
		int			timerEvKey;
		int			signalEvKey;
	};


	//抽象事件(资源类)
	class Event
	{
	public:
		Event() {}
		virtual ~Event() = 0;


		virtual void Handle() = 0;

		void SetEv(uint32 ev) { m_ev = ev; }
		uint32 GetEv() const { return m_ev; }

		void SetHandler(EventHandler* pHandler) { m_pHandler = pHandler; }

		void Handle() { if (!m_pHandler) return; m_pHandler->Handle(this); }

		const EventKey& GetEvKey() const { return m_evKey; }

	private:
		uint32 m_ev;
		EventHandler* m_pHandler;
		EventKey	m_evKey;
	};


	//抽象事件处理器
	class EventHandler
	{
	public:
		EventHandler() {}
		virtual ~EventHandler() = 0;

		virtual void Handle(Event* pEv) = 0;
	};


	//事件的注册、销毁、分发
	class EventCentre
	{
	public:
		EventCentre();
		~EventCentre();

		int Init();

		void EventLoop();

		int RegisterEvent(Event* ev, EventHandler* pHandler);

		int CancelEvent(Event* ev);

		int DispatchEvent();

		int NetEventDispatch();

		int SignalDispatch();

		int TimerDispatch();

		int ProcessActiveEvent();

	private:
		NetEventMap m_netEvs;			//IOMasterEvent->AllIOEvent

		NetDrive* m_pNetDrive;

		TimerEventMap m_timerEvs;

		SignalEventMap m_signalEvs;

		ActiveEventList m_activeEvs;
	};



	////网络事件调度器
	//class NetEventDispatcher
	//{
	//public:
	//	NetEventDispatcher();
	//	virtual ~NetEventDispatcher() = 0;

	//	virtual void Init() = 0;
	//};


	class NetEvent : public Event
	{
	public:
		NetEvent(Socket* pSocket):m_pSocket(pSocket)
		{ 
		}

		~NetEvent()
		{
			if (m_pSocket)
				delete m_pSocket;
		}

		virtual void Handle() override;

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