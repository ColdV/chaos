#pragma once


#include "stdafx.h"
#include "common.h"
#include <functional>
#include <memory>
#include <atomic>
#include <assert.h>
#include "Socket.h"
#include "Buffer.h"
#include "IOCP.h"
#include "thread/Mutex.h"
#include "thread/Condition.h"
#include "thread/ThreadPool.h"


#define SOCKET_ERR_TRY_AGAIN(e) \
(e == EINTR || e == EWOULDBLOCK || e == EAGAIN)

#define SOCKET_ERR_NOT_TRY_AGAIN(e) \
(e != EINTR && e != EWOULDBLOCK && e != EAGAIN)


#define MAX_SINGLE_READ_DEFAULT 16384			//最大单次读入数量
#define MAX_SINGLE_WRITE_DEFAULT 16384			//最大单次写入数量

enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
	EV_SIGNAL = 1 << 4,
	EV_CANCEL = 1 << 5,
	EV_ERROR = 1 << 6,
};


//可设置的SOCK/TCP选项
enum
{
	TCP_OPT_TCP_NODELAY = 1,
};


enum EVENT_UPDATE_OP
{
	EV_CTL_ADD = 1,
	EV_CTL_DEL,
};


namespace chaos
{
	inline int GetLastErrorno()
	{
#ifdef _WIN32
		return WSAGetLastError();
#else
		return errno;
#endif // _WIN32

	}
}


namespace chaos
{
	class Event;
	class EventCentre;
	class Listener;
	class Connecter;
	class Timer;
	class IOCP;


	union EventKey
	{
		socket_t	fd;
		timer_id	timerId;
	};


	union SockAddr
	{
		sockaddr sa;
		sockaddr_in sin;
		sockaddr_in6 sin6;
	};


	const int BASE_CARE_EVENT = EV_CANCEL;
	const int IO_CARE_EVENT = EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT;
	const int TIMEOUT_CART_EVENT = EV_TIMEOUT;


	typedef std::vector<Event*> EventList;
	typedef std::shared_ptr<Event> EventSharedPtr;
	typedef std::weak_ptr<Event> EventWeakPtr;


	//事件
	class Event : public NonCopyable
	{
	public:
		friend class EventCentre;
		friend class Poller;

		typedef std::function<void(Event& pEv, short ev, int errcode)> EventErrCallback;
		typedef std::function<void()> RegisterCallback;

		virtual ~Event() {}

		//响应事件处理
		virtual void Handle() = 0;

		short GetEv() const { return m_ev; }

		short GetCurEv() { if (m_curEv.empty()) return 0; return m_curEv.front(); }

		const EventKey& GetEvKey() const { return m_evKey; }

		EventCentre* GetCentre() const { return m_pCentre; }

		void CancelEvent();

		void SetErrCallback(const EventErrCallback& errcb) { m_errcb = errcb; }

	protected:
		Event(short ev, const EventKey& evKey);

		void PushEventToCentre(short ev);

		void SetEv(short ev) { m_ev = ev; }

		void UpdateEvent(short op, short ev);

		void CallErr(int errcode);

		void SetRegisterCallback(const RegisterCallback& cb) { m_registercb = cb; }

	private:
		void PushCurEv(short ev) { m_curEv.push(ev); }

		void PopCurEv() { if (!m_curEv.empty()) m_curEv.pop(); }

		void SetCenter(EventCentre* pCentre) { m_pCentre = pCentre; }

		void SetEvKey(const EventKey& evKey) { memcpy(&m_evKey, &evKey, sizeof(evKey)); }

	private:
		EventCentre* m_pCentre;			//所属的事件中心

		std::atomic<short> m_ev;		//注册的事件
		
		std::queue<short> m_curEv;		//当前发生的事件队列

		EventKey m_evKey;

		EventErrCallback m_errcb;

		RegisterCallback m_registercb;
	};



	//事件中心(事件循环主体)
	class EventCentre : public NonCopyable
	{
	public:
		friend void Event::PushEventToCentre(short ev);		//use PushEvent();
		friend class IOCP;			//use GetMutex(), WaitWaittintEvsCond()

		struct ActiveEvent
		{
			short ev;
			Event* pEvent;
		};
		
	public:
		EventCentre();
		~EventCentre();

		bool Init();

		int EventLoop();

		void Exit();

		int RegisterEvent(const EventSharedPtr& pEvent);

		void CancelEvent(Event* pEvent);

		//更新已注册的事件
		//@op:更新操作enum EVENT_UPDATE_OP
		//@ev:需要更新的事件
		void UpdateEvent(Event* pEvent, short op, short ev);

		int GetEventsNum() const { return m_eventsNum; }

		//唤醒处于阻塞的centre
		void WakeUp();

	private:
		class WakeUpEvent : public Event
		{
		public:
			explicit WakeUpEvent(socket_t fd);
			~WakeUpEvent();

			bool Init();

			void Handle() override;
				
			bool WakeUp();

		private:
			std::vector<std::unique_ptr<Socket>> m_sockets;
		};

	private:
		void PushEvent(Event* pEvent, short ev);

		int DeleteEvent(Event* pEvent);

		int ProcessActiveEvent();

		//清除所有注册事件和活动事件
		void ClearAllEvent();
	
		//计算当前等待IO的timeout
		int CalculateTimeoutMs();

		//将等到中的事件移动到活动列表
		void MakeWaittingToActive();

		void WaitWaittintEvsCond(int timeoutMs = -1) { MutexGuard lock(m_mutex); m_waittintEvsCond.CondWait(timeoutMs); }

		Mutex& GetMutex() { return m_mutex; }

	private:
		std::unique_ptr<Poller> m_pPoller;				//网络事件调度器

		std::unique_ptr<Timer> m_pTimer;				//定时器

		EventList m_activeEvs;			//活动事件

		EventList m_waittingEvs;		//等待中的事件

		std::atomic_bool m_running;

		std::atomic<int> m_eventsNum;

		bool m_isInit;

		mutable Mutex m_mutex;

		Condition m_waittintEvsCond;

		std::shared_ptr<WakeUpEvent> m_wakeUpEvent;
	};



	class EventCentrePool
	{
	public:
		typedef std::function<void(EventCentrePool*)> StartedCallBack;

		explicit EventCentrePool(int threadNum = 0);
		~EventCentrePool();

		//如果沒设置CallBack,该调用为阻塞模式(直到有所工作线程启动)
		//如果设置CallBack,该调用为非阻塞模式(所有工作线程启动后回调CallBack)
		void Start(const StartedCallBack& cb = NULL);

		void Stop();

		bool Started() const { return m_threadNum; }

		//选择负载较低的事件中心注册事件
		//int RegisterEvent(Event* pEvent);
		int RegisterEvent(const EventSharedPtr& pEvent);

	private:
		void Work();

	private:
		const int m_threadNum;

		Mutex m_mutex;

		std::atomic<bool> m_started;

		Condition m_cond;

		std::unique_ptr<ThreadPool> m_pool;

		std::vector<EventCentre*> m_centres;

		StartedCallBack m_startedCb;
	};



	class Listener :public Event
	{
	public:
#ifdef IOCP_ENABLE
		static const int INIT_ASYNACCEPTING			= 8;
		static const int INIT_ACCEPTADDRBUF_SIZE	= 128; //AcceptEx传入的buffer大小(传输协议的最大地址大小 + 16) * 2
#endif // IOCP_ENABLE

		typedef std::function<void(Listener& listener, socket_t acceptedfd)>	ListenerCb;

		Listener(socket_t fd, const SockAddr& addr);

		~Listener();

		static std::shared_ptr<Listener> CreateListener(const char* ip = NULL, unsigned short port = 0,
				bool ipv6 = false, int opt = 0, const ListenerCb& cb = NULL);

		Socket& GetSocket() { return *m_socket; }

		void SetListenerCb(const ListenerCb& cb) { m_cb = cb; }

		void CallListenerCb(socket_t acceptedfd) { if (m_cb) m_cb(*this, acceptedfd); }

	private:
		virtual void Handle() override;

		int Listen();

		void DoneAccept(socket_t acceptedfd);

		void RegisterCallback();

#ifdef IOCP_ENABLE
		int StartAsynAccept();

		int AsynAccept(LPACCEPT_OVERLAPPED lo);

		//GetQueuedCompletionStatus后的回调
		void AcceptComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk);

#endif // IOCP_ENABLE

	private:
		//Socket* m_socket;
		std::unique_ptr<Socket> m_socket;

		ListenerCb m_cb;

		SockAddr m_addr;

#ifdef IOCP_ENABLE
		LPACCEPT_OVERLAPPED m_acceptOls;

		int* m_overlappedsRefCnt;

		char** m_acceptBuffers;

		std::queue<LPACCEPT_OVERLAPPED> m_acceptedq;
#endif // IOCP_ENABLE
	};



	class Connecter : public Event
	{
	public:
		typedef std::function<void(Connecter& pConnect, int nTransBytes)> NetCallback;

		explicit Connecter(socket_t fd);

		~Connecter();

		Socket& GetSocket() { return *m_socket; }

		int Connect(sockaddr* sa, int salen);

		//将buffer中的len个长度的数据写入到内部buffer中, 并且推送一个EV_IOWRITE事件在下一帧写入socket
		int WriteBuffer(const char* buffer, int len);

		//写入socket
		int Send(const char* buf, int size);

		//从RBuffer中读取len个字节到buf中
		//return:返回实际读取的字节数
		int ReadBuffer(char* buf, int size);

		//获取RBuffer中可读取的字节数
		int GetReadableSize() { if (!m_pReadBuffer) return 0; return m_pReadBuffer->GetReadSize(); }

		void SetReadCallback(const NetCallback& cb) { m_readcb = cb; }

		void SetWriteCallback(const NetCallback& cb) { m_writecb = cb; }

		void SetConnectCallback(const NetCallback& cb) { m_connectcb = cb; }

		//启用一个新事件
		void EnableEvent(short ev);

		//关闭关注的事件
		void DisableEvent(short ev);

		bool Connected() const { return m_connected; }

#ifdef IOCP_ENABLE
		int AsyncConnect(sockaddr* sa, int salen);
#endif // IOCP_ENABLE


	private:
		virtual void Handle() override;

		void CallbackRead(int nTransferBytes) { if (m_readcb) m_readcb(*this, nTransferBytes); }

		void CallbackWrite(int nTransferBytes) { if (m_writecb) m_writecb(*this, nTransferBytes); }

		void CallbackConnect(bool bOk) { if (m_connectcb) m_connectcb(*this, bOk); }

		void RegisterCallback();

		int HandleRead();

		int HandleWrite();

#ifdef IOCP_ENABLE
		int AsynRead();

		int AsynWrite();

		void ReadComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk);

		void WriteComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk);

		void ConnectComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk);
#endif // IOCP_ENABLE

	private:
		//Socket* m_socket;
		std::unique_ptr<Socket> m_socket;

		Buffer* m_pReadBuffer;

		Buffer* m_pWriteBuffer;

#ifdef IOCP_ENABLE
		LPCOMPLETION_OVERLAPPED m_pReadOverlapped;
		bool m_isPostRecv;			//是否已投递WSARecv事件(IOCP在等待事件完成)

		LPCOMPLETION_OVERLAPPED m_pWriteOverlapped;
		bool m_isPostWrite;			//是否已投递WSASend事件(IOCP在等待事件完成)

		LPCOMPLETION_OVERLAPPED m_pConnectOverlapped;
		bool m_isPostConnect;		//是否已投递ConnectEx事件(IOCP在等待事件完成)

#endif // IOCP_ENABLE

		NetCallback m_readcb;

		NetCallback m_writecb;

		NetCallback m_connectcb;

		SockAddr m_peeraddr;

		bool m_connected;
	};



	class TimerEvent : public Event
	{
	public:
		friend class Timer;
		typedef std::function<void(TimerEvent&)> TimerHandler;

		explicit TimerEvent(uint32 timeoutMs, const TimerHandler& func = NULL, bool isLoop = false);

		virtual ~TimerEvent();

		uint32 GetTimeOut() const { return m_timeoutMs; }

		time_t GetNextTime() const { return m_nextTime; }

		bool IsLoop() const { return m_isLoop; }
		void SetLoop(bool isLoop) { m_isLoop = isLoop; }

		//暂停定时器
		void Suspend() { m_isSuspend = true; };

		//重启定时器
		void Resume() { m_isSuspend = false; }

		bool IsSuspend() const { return m_isSuspend; }

		void SetTimerHandle(const TimerHandler& func) { m_handleFunc = func; };

	private:
		virtual void Handle() override;

		void SetNextTime() { m_nextTime = GetCurrentMSec() + m_timeoutMs; }

	private:
		uint32 m_timeoutMs;

		uint64 m_nextTime;

		bool m_isLoop;						//循环定时器

		bool m_isSuspend;					//暂停定时器

		TimerHandler m_handleFunc;
	};


}	//namespace chaos