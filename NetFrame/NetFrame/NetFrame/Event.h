#pragma once


#include "../../common/stdafx.h"
#include <map>
#include <set>
#include "NetDrive.h"
#include "Buffer.h"
#include "IOCP.h"

enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
	EV_SIGNAL = 1 << 4,
};


namespace NetFrame
{
	//class EventHandler;
	class Event;
	class EventCentre;
	class Timer;


	union EventKey
	{
		socket_t	fd;
		timer_id	timerId;
		int			signal;
	};


	struct EvAndKey
	{
		uint32 ev;
		EventKey key;
	};

	//抽象事件(资源类)
	class Event : public NonCopyable
	{
	public:
		virtual ~Event()
		{
			if (m_pEvKey)
				delete m_pEvKey;
		}

		virtual void Handle() = 0;

		short GetEv() const { return m_ev; }

		short GetCurEv() const { return m_curEv; }

		void SetCurEv(short ev) { m_curEv = ev; }

		const EventKey* GetEvKey() const { return m_pEvKey; }

		EventCentre* GetCentre() const { return m_pCenter; }

	protected:
		Event(EventCentre* pCentre, short ev, EventKey* pEvKey) :
			m_ev(ev),
			m_curEv(0),
			m_pEvKey(pEvKey),
			m_pCenter(pCentre)
		{
		}


		void CancelEvent();

	private:
		EventCentre* m_pCenter;		//所属的事件中心
		short m_ev;		//注册的事件
		short m_curEv;		//当前发生的事件
		EventKey*	m_pEvKey;
	};


	//事件的注册、销毁、分发
	class EventCentre : public NonCopyable
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

		int RegisterEvent(Event* pEvent);

		int CancelEvent(Event* pEvent);

		int DispatchEvent();

		void PushActiveEv(Event* pEvent) { m_activeEvs.push_back(pEvent); }

	private:
		int NetEventDispatch();

		int SignalDispatch();

		int TimerDispatch();

		int ProcessActiveEvent();

	private:
		//NetEventMap m_netEvs;			//IOMasterEvent->AllIOEvent

		NetDrive* m_pNetDrive;

		TimerEventMap m_timerEvs;

		Timer* m_pTimer;

		SignalEventMap m_signalEvs;

		ActiveEventList m_activeEvs;

		bool m_running;
	};


	class NetEvent :public Event
	{
	public:
		NetEvent(EventCentre* pCentre, Socket* pSocket, short ev, EventKey* pEvKey) :
			Event(pCentre, ev, pEvKey),
			m_pSocket(pSocket)
		{
		}

		virtual ~NetEvent()
		{
			if (m_pSocket)
				delete m_pSocket;
		}

		Socket* GetSocket() const { return m_pSocket; }

		//virtual void Handle() override;

	protected:
		Socket* m_pSocket;
	};


	//可以再封装一个 Connecter 和 Listener 一起继承自NetEvent
	class Listener :public NetEvent
	{
	public:
		Listener(EventCentre* pCentre, Socket* pSocket, short ev, EventKey* pEvKey) :
			NetEvent(pCentre, pSocket, ev,pEvKey)
		{
		}

		~Listener()
		{
			if (m_pSocket)
				delete m_pSocket;
		}

		virtual void Handle() override;
	};


	class Connecter : public NetEvent
	{
	public:
		Connecter(EventCentre* pCentre, Socket* pSocket, short ev, EventKey* pEvKey):
			NetEvent(pCentre, pSocket, ev, pEvKey)
		{
			m_pRBuffer = new Buffer;
			m_pWBuffer = new Buffer;
		}

		~Connecter()
		{
			if (m_pRBuffer)
				delete m_pRBuffer;

			if (m_pWBuffer)
				delete m_pWBuffer;
		}

		virtual void Handle() override;

	private:
		//int HandleListen();

		int HandleRead();

		int HandleWrite();

	private:
		Buffer* m_pRBuffer;
		Buffer* m_pWBuffer;

	};


#ifdef WIN32
	//异步IO事件(IOCP)
	class AsynConnecter : public NetEvent
	{
	public:
		AsynConnecter(EventCentre* pCentre, Socket* pSocket, short ev, EventKey* pEvKey) :
			NetEvent(pCentre, pSocket, ev, pEvKey)
		{
			m_pRBuffer = new Buffer;
			m_pWBuffer = new Buffer;
			m_pOverlapped = new COMPLETE_OVERLAPPED_DATA;

			if (m_pOverlapped)
			{
				memset(m_pOverlapped, 0, sizeof(COMPLETE_OVERLAPPED_DATA));
				m_pOverlapped->asynRet = INVALID_IOCP_RET;
			}
		}

		~AsynConnecter()
		{
			if (m_pRBuffer)
				delete m_pRBuffer;

			if (m_pWBuffer)
				delete m_pWBuffer;

			if (m_pOverlapped)
				delete m_pOverlapped;
		}

		virtual void Handle() override;

	private:
		int AsynRead();

		int AsynWrite();

	private:
		Buffer* m_pRBuffer;
		Buffer* m_pWBuffer;
		LPCOMPLETE_OVERLAPPED_DATA m_pOverlapped;
	};
#endif // WIN32




	class TimerEvent : public Event
	{
	public:
		TimerEvent(EventCentre* pCentre, uint32 ev, EventKey* pEvKey, uint32 timeOut, bool isLoop = false) :
			Event(pCentre, ev, pEvKey),
			m_timeOut(timeOut),
			m_isLoop(isLoop)
		{
		}

		virtual ~TimerEvent()
		{}

		uint32 GetTimeOut() const { return m_timeOut; }

		bool IsLoop() const { return m_isLoop; }
		void SetLoop(bool isLoop) { m_isLoop = isLoop; }

		virtual void Handle() override;

	private:
		uint32 m_timeOut;
		bool m_isLoop;
	};


}	//namespace NetFrame