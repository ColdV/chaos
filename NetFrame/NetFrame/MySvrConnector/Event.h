#pragma once


#include "../common/stdafx.h"
#include <map>
#include <set>
#include "NetDrive.h"
#include "Buffer.h"

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
	class EventCentre;


	//抽象事件处理器
	class EventHandler
	{
	public:
		EventHandler() {}
		virtual ~EventHandler() = 0 {};

		virtual void Handle(Event* pEv) = 0;
	};


	union EventKey
	{
		socket_t	fd;
		int			timerId;
		int			signal;
	};


	//抽象事件(资源类)
	class Event
	{
	public:
		/*Event() {}
		virtual ~Event() = 0;*/

		virtual void Handle() = 0;

		uint32 GetEv() const { return m_ev; }

		/*void SetHandler(EventHandler* pHandler) { m_pHandler = pHandler; }*/

		uint32 GetCurEv() const { return m_curEv; }
		void SetCurEv(uint32 ev) { m_curEv = ev; }

		//void Handle() { if (!m_pHandler) return; m_pHandler->Handle(this); }

		const EventKey* GetEvKey() const { return m_pEvKey; }


	protected:
		Event(EventCentre* pCentre, uint32 ev, EventHandler* pHandler, EventKey* pEvKey) :
			m_ev(ev),
			m_curEv(0),
			m_pHandler(pHandler),
			m_pEvKey(pEvKey),
			m_pCenter(pCentre)
		{
		}

		virtual ~Event() = 0
		{
			if (m_pHandler)
				delete m_pHandler;

			if (m_pEvKey)
				delete m_pEvKey;
		}

		//int AddNewEvent(Event* pNewEv);

		EventCentre* GetCentre() const { return m_pCenter; }

	private:
		EventCentre* m_pCenter;		//所属的事件中心
		uint32 m_ev;		//注册的事件
		uint32 m_curEv;		//当前发生的事件
		EventHandler* m_pHandler;
		EventKey*	m_pEvKey;
	};


	//事件的注册、销毁、分发
	class EventCentre
	{
	public:
		typedef std::map<socket_t, Event*> NetEventMap;
		typedef std::map<int, Event*>	TimerEventMap;
		typedef std::map<int, Event*>	SignalEventMap;
		typedef std::list<Event*>	ActiveEventList;


	public:
		EventCentre();
		~EventCentre();

		int Init();

		void EventLoop();

		int RegisterEvent(Event* ev, EventHandler* pHandler);

		int CancelEvent(Event* ev);

		int DispatchEvent();

	private:
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




	class NetEvent : public Event
	{
	public:
		NetEvent(EventCentre* pCentre, Socket* pSocket, uint32 ev, EventHandler* pHandler, EventKey* pEvKey):
			Event(pCentre, ev, pHandler, pEvKey),
			m_pSocket(pSocket)
		{
			m_pRBuffer = new Buffer;
			m_pWBuffer = new Buffer;
		}

		~NetEvent()
		{
			if (m_pSocket)
				delete m_pSocket;

			if (m_pRBuffer)
				delete m_pRBuffer;

			if (m_pWBuffer)
				delete m_pWBuffer;
		}

		Socket* GetSocket() const { return m_pSocket; }

		virtual void Handle() override;

	private:
		int HandleListen();

		int HandleRead();

		int HandleWrite();

	private:
		Socket* m_pSocket;
		Buffer* m_pRBuffer;
		Buffer* m_pWBuffer;
	};


	class TimerEvent : public Event
	{
	public:
		TimerEvent(EventCentre* pCentre, uint32 ev, EventKey* pEvKey, uint32 timeOut):
			Event(pCentre, ev, NULL, pEvKey),
			m_timeOut(timeOut)
		{
		}

		virtual ~TimerEvent()
		{}

		uint32 GetTimeOut() const { return m_timeOut; }

		virtual void Handle() override;

	private:
		uint32 m_timeOut;
		bool m_isLoop;
	};


	class NetEventHandler : public EventHandler
	{
	public:
		NetEventHandler() {}
		~NetEventHandler() {}

		virtual void Handle(Event* pEv) override;


	private:
		int HandleListen(NetEvent* pEv);

		int HandleRead(NetEvent* pEv);

		int HandleWrite(NetEvent* pEv);
	};

}	//namespace NetFrame