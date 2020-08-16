#pragma once


#include "../../common/stdafx.h"
#include <map>
#include <set>
#include <functional>
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
	class Listener;
	class Connecter;
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
		friend class EventCentre;
		friend class Poller;

		typedef std::function<void(Event* pEv, short ev, void* arg)> EventCallback;

		virtual ~Event()
		{
		}

		virtual void Handle() = 0;

		short GetEv() const { return m_ev; }

		void SetEv(short ev) { m_ev = ev; }

		short GetCurEv() const { return m_curEv; }

		const EventKey& GetEvKey() const { return m_evKey; }

		EventCentre* GetCentre() const { return m_pCenter; }

		void SetEventCallback(EventCallback cb, void* pUserData) { m_callback = cb; m_cbArg = pUserData; }

		void Callback() { if (m_callback) m_callback(this, m_curEv, m_cbArg); }

	protected:
		Event(EventCentre* pCentre, short ev, const EventKey& evKey);
		Event();

		void CancelEvent();

	private:
		void SetCurEv(short ev) { m_curEv = ev; }

		void SetCenter(EventCentre* pCentre) { m_pCenter = pCentre; }

		void SetEvKey(const EventKey& evKey) { memcpy(&m_evKey, &evKey, sizeof(evKey)); }

	private:
		EventCentre* m_pCenter;		//所属的事件中心

		short m_ev;					//注册的事件

		short m_curEv;				//当前发生的事件

		EventKey	m_evKey;

		EventCallback m_callback;

		void* m_cbArg;
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

		int EventLoop(int loopTickTimeMs = 0);

		int RegisterEvent(Event* pEvent);

		int CancelEvent(Event* pEvent);

		void PushActiveEv(Event* pEvent) { m_mutex.Lock(); m_activeEvs.push(pEvent); m_mutex.UnLock(); }

		void PushActiveEv(Event* pEvent, short ev);

	private:
		//int Dispatch();

		//int NetEventDispatch();

		int SignalDispatch();

		//int TimerDispatch();

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

		int m_evcount;

		bool m_isInit;

		Mutex m_mutex;
	};


	class NetEvent :public Event
	{
	public:
		NetEvent(EventCentre* pCentre, short ev, socket_t fd) :
			Event(pCentre, ev, (EventKey&)fd),
			m_socket(fd)
		{}


		virtual ~NetEvent()
		{
		}

		Socket& GetSocket() { return m_socket; }

	protected:
		Socket m_socket;
	};


	//可以再封装一个 Connecter 和 Listener 一起继承自NetEvent
	class Listener :public NetEvent
	{
	public:
		static const int INIT_ASYNACCEPTING = 8;
		static const int INIT_ACCEPTADDRBUF = 256;

		typedef std::function<void(Listener* pListener, Connecter* pConnecter, void* arg)>	ListenerCb;

		Listener(EventCentre* pCentre, socket_t fd) :
			NetEvent(pCentre, EV_IOREAD, fd),
			m_cbArg(0)
		{
#ifdef _WIN32
			m_acceptOls = new ACCEPT_OVERLAPPED[INIT_ASYNACCEPTING];
			if (!m_acceptOls)
				return;

			m_acceptBuffers = new Buffer[INIT_ASYNACCEPTING];
			if (!m_acceptBuffers)
				return;

			for (int i = 0; i < INIT_ASYNACCEPTING; ++i)
			{
				memset(&m_acceptOls[i], 0, sizeof(ACCEPT_OVERLAPPED));

				m_acceptBuffers[i].Reserver(INIT_ACCEPTADDRBUF);

				//acceptex只使用一个数据块 用于存放连接后的地址信息
				//之后可通过GetAcceptExSockaddrs获得本地或远程的地址
				uint32 n = 0;
				m_acceptOls[i].overlapped.databufs[0].buf = m_acceptBuffers[i].GetWriteBuffer(&n);
				m_acceptOls[i].overlapped.databufs[0].len = n;

				m_acceptOls[i].acceptfd = INVALID_SOCKET;
				m_acceptOls[i].overlapped.fd = INVALID_SOCKET;

				m_acceptOls[i].overlapped.cb = std::bind(&Listener::IocpListenCallback, this, std::placeholders::_1, std::placeholders::_2, 
					std::placeholders::_3, std::placeholders::_4);
			}

#endif // _WIN32
		}


		~Listener()
		{
#ifdef _WIN32
			if (m_acceptOls)
				delete[] m_acceptOls;

			if (m_acceptBuffers)
				delete[] m_acceptBuffers;
#endif // _WIN32

		}

		int Listen(const sockaddr* sa, int salen);

		virtual void Handle() override;

		void SetListenerCb(ListenerCb cb, void* pCbData) { m_cb = cb; m_cbArg = pCbData; }

		void CallListenerCb(Connecter* pConner) { if (m_cb) m_cb(this, pConner, m_cbArg); }

	private:
#ifdef _WIN32
		int AsynAccept(LPACCEPT_OVERLAPPED lo);

		//GetQueuedCompletionStatus后的回调
		void IocpListenCallback(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok);
#endif // _WIN32

	private:
		ListenerCb m_cb;

		void* m_cbArg;

#ifdef _WIN32
		LPACCEPT_OVERLAPPED m_acceptOls;

		Buffer* m_acceptBuffers;

		std::queue<LPACCEPT_OVERLAPPED> m_acceptedq;
#endif // _WIN32

	};


	class Connecter : public NetEvent
	{
	public:
		typedef std::function<void(Connecter* pConnect, int nTransBytes, void* arg)> NetCallback;

		Connecter(EventCentre* pCentre, socket_t fd):
			NetEvent(pCentre, EV_IOREAD | EV_IOWRITE, fd)
		{
			m_pRBuffer = new Buffer;
			m_pWBuffer = new Buffer;

#ifdef _WIN32
			m_pROverlapped = new COMPLETION_OVERLAPPED;

			if (m_pROverlapped)
			{
				memset(m_pROverlapped, 0, sizeof(COMPLETION_OVERLAPPED));
				//m_pROverlapped->asynRet = INVALID_IOCP_RET;
				m_pROverlapped->cb = std::bind(&Connecter::IocpReadCallback, this, std::placeholders::_1, std::placeholders::_2,
					std::placeholders::_3, std::placeholders::_4);
			}

			m_pWOverlapped = new COMPLETION_OVERLAPPED;
			if (m_pWOverlapped)
			{
				memset(m_pWOverlapped, 0, sizeof(COMPLETION_OVERLAPPED));
				//m_pWOverlapped->asynRet = INVALID_IOCP_RET;
				m_pWOverlapped->cb = std::bind(&Connecter::IocpWriteCallback, this, std::placeholders::_1, std::placeholders::_2,
					std::placeholders::_3, std::placeholders::_4);
			}
#endif // WIN32
		}

		~Connecter()
		{
			if (m_pRBuffer)
				delete m_pRBuffer;

			if (m_pWBuffer)
				delete m_pWBuffer;

#ifdef _WIN32
			if (m_pROverlapped)
				delete m_pROverlapped;

			if (m_pWOverlapped)
				delete m_pWOverlapped;
#endif // _WIN32
		}

		virtual void Handle() override;

		//讲数据写入到buff中, 并且推送一个EV_IOWRITE事件在下一帧写入socket
		int WriteBuffer(const char* buf, int len);

		//写入socket
		int Write(const char* buf, int size);

		//从RBuffer中读取len个字节到buf中
		//return:返回实际读取的字节数
		int ReadBuffer(char* buf, int size);

		//获取RBuffer中可读取的字节数
		int GetReadableSize() { if (!m_pRBuffer) return 0; return m_pRBuffer->GetReadSize(); }

		void SetCallback(NetCallback readcb, void* readCbArg, NetCallback writecb, void* writeCbArg)
		{
			m_readcb = readcb;
			m_readCbArg = readCbArg;
			m_writecb = writecb;
			m_writeCbArg = writeCbArg;
		}

		void SetReadCallback(NetCallback readcb, void* readCbArg)
		{
			m_readcb = readcb;
			m_readCbArg = readCbArg;
		}

		void SetWriteCallback(NetCallback writecb, void* writeCbArg)
		{
			m_writecb = writecb;
			m_writeCbArg = writeCbArg;
		}


	private:
		void CallbackRead(int nTransferBytes) { if (m_readcb) m_readcb(this, nTransferBytes, m_readCbArg); }

		void CallbackWrite(int nTransferBytes) { if (m_writecb) m_writecb(this, nTransferBytes, m_writeCbArg); }


		int HandleRead();

		int HandleWrite();

#ifdef _WIN32
		int AsynRead();

		int AsynWrite();

		void IocpReadCallback(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok);

		void IocpWriteCallback(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok);
#endif // _WIN32

	private:
		Buffer* m_pRBuffer;

		Buffer* m_pWBuffer;

#ifdef _WIN32
		LPCOMPLETION_OVERLAPPED m_pROverlapped;

		LPCOMPLETION_OVERLAPPED m_pWOverlapped;
#endif // _WIN32

		NetCallback m_readcb;

		void* m_readCbArg;

		NetCallback m_writecb;

		void* m_writeCbArg;


		Mutex m_mutex;
	};


//#ifdef _WIN32
//	//异步IO事件(IOCP)
//	class AsynConnecter : public NetEvent
//	{
//	public:
//		AsynConnecter(EventCentre* pCentre, socket_t fd) :
//			NetEvent(pCentre, EV_IOREAD | EV_IOWRITE, fd)
//		{
//			m_pRBuffer = new Buffer;
//			m_pWBuffer = new Buffer;
//			m_pOverlapped = new COMPLETION_OVERLAPPED;
//
//			if (m_pOverlapped)
//			{
//				memset(m_pOverlapped, 0, sizeof(COMPLETION_OVERLAPPED));
//				m_pOverlapped->asynRet = INVALID_IOCP_RET;
//			}
//		}
//
//		~AsynConnecter()
//		{
//			if (m_pRBuffer)
//				delete m_pRBuffer;
//
//			if (m_pWBuffer)
//				delete m_pWBuffer;
//
//			if (m_pOverlapped)
//				delete m_pOverlapped;
//		}
//
//		virtual void Handle() override;
//
//		int WriteBuffer(const char* buf, int len) override { return 0; };
//
//		int Write(const char* buf, int len) override { return 0; };
//
//		int ReadBuffer(char* buf, int len) override { return 0; };
//
//	private:
//		int AsynRead();
//
//		int AsynWrite();
//
//	private:
//		Buffer* m_pRBuffer;
//		Buffer* m_pWBuffer;
//		LPCOMPLETION_OVERLAPPED m_pOverlapped;
//		Mutex m_mutex;
//	};
//#endif // _WIN32




	class TimerEvent : public Event
	{
	public:
		friend class Timer;

		TimerEvent(EventCentre* pCentre, timer_id timerId, uint32 timeout, bool isLoop = false) :
			Event(pCentre, EV_TIMEOUT, (EventKey&)timerId),
			m_timeout(timeout),
			m_isLoop(isLoop)
		{
			m_nextTime = time(NULL) + m_timeout;
		}

		virtual ~TimerEvent()
		{}

		uint32 GetTimeOut() const { return m_timeout; }

		time_t GetNextTime() const { return m_nextTime; }

		bool IsLoop() const { return m_isLoop; }
		void SetLoop(bool isLoop) { m_isLoop = isLoop; }

		virtual void Handle() override;

	private:
		void SetNextTime() { m_nextTime = time(NULL) + m_timeout; }

	private:
		uint32 m_timeout;

		time_t m_nextTime;

		bool m_isLoop;
	};


}	//namespace chaos