#pragma once


#include "../../common/stdafx.h"
#include <map>
#include <set>
#include "Poller.h"
#include "Buffer.h"
#include "IOCP.h"
#include "../thread/Mutex.h"

enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
	EV_SIGNAL = 1 << 4,
};


namespace chaos
{
	class Event;
	class EventCentre;
	class Timer;


	union EventKey
	{
		socket_t	fd;
		timer_id	timerId;
		int			signal;
	};



	//事件
	class Event : public NonCopyable
	{
	public:
		virtual ~Event()
		{
		}

		virtual void Handle() = 0;

		short GetEv() const { return m_ev; }

		short GetCurEv() const { return m_curEv; }

		void SetCurEv(short ev) { m_curEv = ev; }

		const EventKey& GetEvKey() const { return m_evKey; }

		EventCentre* GetCentre() const { return m_pCenter; }

	protected:
		Event(EventCentre* pCentre, short ev, const EventKey& evKey);
		Event();

		void CancelEvent();

		void SetCenter(EventCentre* pCentre) { m_pCenter = pCentre; }

		void SetEv(short ev) { m_ev = ev; }

		void SetEvKey(const EventKey& evKey) { memcpy(&m_evKey, &evKey, sizeof(evKey)); }

	private:
		EventCentre* m_pCenter;		//所属的事件中心
		short m_ev;					//注册的事件
		short m_curEv;				//当前发生的事件
		EventKey	m_evKey;
	};


	//事件的注册、销毁、分发
	class EventCentre : public NonCopyable
	{
	public:
		typedef std::map<socket_t, Event*> NetEventMap;
		/*typedef std::map<int, Event*>	TimerEventMap;*/
		typedef std::map<int, Event*>	SignalEventMap;
		typedef std::list<Event*>	ActiveEventList;
		typedef std::queue<Event*>	EvQueue;


	public:
		EventCentre();
		~EventCentre();

		int Init();

		void EventLoop();

		int RegisterEvent(Event* pEvent);

		int CancelEvent(Event* pEvent);

		int DispatchEvent();

		void PushActiveEv(Event* pEvent) { m_mutex.Lock(); m_activeEvs.push(pEvent); m_mutex.UnLock(); }

	private:
		int NetEventDispatch();

		int SignalDispatch();

		int TimerDispatch();

		int ProcessActiveEvent();

	private:
		//NetEventMap m_netEvs;			//IOMasterEvent->AllIOEvent

		Poller* m_pPoller;				//网络事件调度器

		//TimerEventMap m_timerEvs;

		Timer* m_pTimer;				//定时器

		SignalEventMap m_signalEvs;

		//ActiveEventList m_activeEvs;
		EvQueue m_activeEvs;

		bool m_running;

		Mutex m_mutex;
	};


	class NetEvent :public Event
	{
	public:
		//NetEvent(EventCentre* pCentre, Socket* pSocket, short ev, const EventKey& evKey) :
		//	Event(pCentre, ev, evKey),
		//	m_pSocket(pSocket)
		//{
		//}

		NetEvent(EventCentre* pCentre, short ev, socket_t fd) :
			Event(pCentre, ev, (EventKey&)fd),
			m_socket(fd)
		{}


		virtual ~NetEvent()
		{
		}

		Socket& GetSocket() { return m_socket; }

		//virtual void Handle() override;

	protected:
		Socket m_socket;
	};


	//可以再封装一个 Connecter 和 Listener 一起继承自NetEvent
	class Listener :public NetEvent
	{
	public:
		Listener(EventCentre* pCentre, socket_t fd) :
			NetEvent(pCentre, EV_IOREAD, fd)
		{}


		~Listener()
		{
		}

		int Listen(const char* ip, int port);

		virtual void Handle() override;
	};


	class Connecter : public NetEvent
	{
	public:
		Connecter(EventCentre* pCentre, socket_t fd):
			NetEvent(pCentre, EV_IOREAD | EV_IOWRITE, fd)
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

		//讲数据写入到buff中, 并且推送一个EV_IOWRITE事件在下一帧写入socket
		int WriteBuffer(const char* buf, int len);

		//写入socket
		int Write(const char* buf, int len);

	private:
		//int HandleListen();

		int HandleRead();

		int HandleWrite();

	private:
		Buffer* m_pRBuffer;
		Buffer* m_pWBuffer;
		Mutex m_mutex;
	};


#ifdef _WIN32
	//异步IO事件(IOCP)
	class AsynConnecter : public NetEvent
	{
	public:
		AsynConnecter(EventCentre* pCentre, socket_t fd) :
			NetEvent(pCentre, EV_IOREAD | EV_IOWRITE, fd)
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

		int AsynWrite(const char* buf, int len);

	private:
		int AsynRead();

		int AsynWrite();

	private:
		Buffer* m_pRBuffer;
		Buffer* m_pWBuffer;
		LPCOMPLETE_OVERLAPPED_DATA m_pOverlapped;
		Mutex m_mutex;
	};
#endif // _WIN32




	class TimerEvent : public Event
	{
	public:
		TimerEvent(EventCentre* pCentre, uint32 ev, const EventKey& evKey, uint32 timeOut, bool isLoop = false) :
			Event(pCentre, ev, evKey),
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


}	//namespace chaos