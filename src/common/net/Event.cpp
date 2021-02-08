#include "Event.h"
#include "Timer.h"
#include <unordered_set>


namespace chaos
{
	Event::Event(short ev, const EventKey& evKey) :
		m_pCentre(NULL),
		m_ev(ev | BASE_CARE_EVENT),
		m_errcb(NULL),
		m_registercb(NULL)
	{
		memcpy(&m_evKey, &evKey, sizeof(EventKey));
	}


	void Event::CancelEvent()
	{
		if (m_pCentre)
		{
			m_pCentre->CancelEvent(this);
		}
	}


	void Event::PushEventToCentre(short ev)
	{
		if (!m_pCentre)
			return;

		m_pCentre->PushEvent(this, ev);
	}


	void Event::CallErr(int errcode)
	{
		if (m_errcb)
			m_errcb(*this, GetCurEv() | EV_ERROR, errcode);
		else
			CancelEvent();
	}


	void Event::UpdateEvent(short op, short ev)
	{
		if (!m_pCentre)
			return;

		if (EV_CTL_ADD == op)
			m_ev |= ev;
		else if (EV_CTL_DEL == op)
			m_ev &= ~ev;

		m_pCentre->UpdateEvent(this, op, ev);
	}

}	//namespace chaos



//EventCentre Implement
namespace chaos
{
	EventCentre::WakeUpEvent::WakeUpEvent(socket_t fd) :
		Event(EV_IOREAD, { fd })
	{
		m_sockets.emplace_back(new Socket(fd));
	}


	EventCentre::WakeUpEvent::~WakeUpEvent()
	{}


	bool EventCentre::WakeUpEvent::Init()
	{
#ifdef _WIN32
#ifndef IOCP_ENABLE
		SOCKADDR_IN sin;
		memset(&sin, 0, sizeof(SOCKADDR_IN));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = 0;
		sin.sin_port = htons(0);

		if (0 != m_sockets[0]->Bind((sockaddr*)&sin, sizeof(SOCKADDR_IN)))
			return false;

		if (0 != m_sockets[0]->Listen(2))
			return false;

		m_sockets.emplace_back(new Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

		return true;
#endif // !IOCP_ENABLE
#endif //  _WIN32

		return true;
	}


	void EventCentre::WakeUpEvent::Handle()
	{
		int ret = 0;
#ifdef _WIN32
#ifndef IOCP_ENABLE
		char buf(0);
		ret = m_sockets[1]->Recv(&buf, sizeof(buf));
#endif // !IOCP_ENABLE
#else
		uint64_t buf = 1;
		ret = read(m_sockets[0]->GetFd(), &buf, sizeof(buf));
	
#endif // _WIN32
		if (ret < 0)
		{
			printf("wake up fd read data failed! errno:%d\n", GetLastErrorno());
		}
	}


	bool EventCentre::WakeUpEvent::WakeUp()
	{
#ifdef _WIN32
#ifndef IOCP_ENABLE
		char buf(0);
		return m_sockets[1]->Send(&buf, sizeof(buf)) < 0 ? false : true;
#endif // !IOCP_ENABLE
		GetCentre()->m_waittintEvsCond.CondBroadCast();
#else	//linux event fd
		uint64_t buf = 1;
		return write(m_sockets[0]->GetFd(), &buf, sizeof(buf)) < 0 ? false : true;
#endif // _WIN32

		return true;	
	}


	EventCentre::EventCentre() :
		m_pPoller(Poller::AdapterNetDrive(this)),
		m_pTimer(new Timer(this)),
		m_running(false),
		m_eventsNum(0),
		m_isInit(false),
		m_waittintEvsCond(m_mutex),
#ifdef _WIN32
		m_wakeUpEvent(new WakeUpEvent(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
#else
		m_wakeUpEvent(new WakeUpEvent(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)))
#endif // _WIN32

	{
		Init();
	}


	EventCentre::~EventCentre()
	{
		//ClearAllEvent();

		//if (m_pPoller)
		//	delete m_pPoller;

		//if (m_pTimer)
		//	delete m_pTimer;
	}


	bool EventCentre::Init()
	{
		if (m_isInit)
			return true;

		if (0 != m_pPoller->Init())
		{
			printf("init poller failed!\n");
			return false;
		}

		if (!m_wakeUpEvent->Init())
		{
			printf("init wake up event failed!\n");
			return false;
		}

		if (0 != RegisterEvent(m_wakeUpEvent))
		{
			printf("register wake up event failed!\n");
			return false;
		}

		m_isInit = true;

		return false;
	}


	int EventCentre::EventLoop()
	{
		if (!m_isInit)
		{
			printf("event centre is not init, please call init function!\n");
			return -1;
		}

		if (m_running)
		{
			printf("event center is running!\n");
			return -1;
		}

		m_running = true;

		while (m_running)
		{
			MakeWaittingToActive();

			int timeoutMs = CalculateTimeoutMs();

			if (0 != m_pPoller->Launch(timeoutMs, m_activeEvs))
			{
				printf("called poller failed!\n");
				break;
			}

			m_pTimer->Launch(m_activeEvs);

			if (0 != ProcessActiveEvent())
			{
				printf("process active event failed!\n");
				break;
			}
		}

		m_running = false;

		//ClearAllEvent();

		printf("EventCentre exit!\n");

		return 0;
	}


	int EventCentre::ProcessActiveEvent()
	{
		MutexGuard lock(m_mutex);

		for(auto pev : m_activeEvs)
		{
			assert(pev);

			//EV_CANCEL时GetCentre已为NULL
			//优先处理CANCEL,这个判断必须在GetCentre之前
			if (pev->GetCurEv() & EV_CANCEL)
			{
				DeleteEvent(pev);
				continue;
			}

			if (!pev->GetCentre())
				continue;

			pev->Handle();

			//清除此次已处理的事件
			pev->PopCurEv();
		}

		m_activeEvs.clear();

		return 0;
	}


	void EventCentre::ClearAllEvent()
	{
		//MutexGuard lock(m_mutex);

		//Poller::NetEventMap& allNetEvent = m_pPoller->GetAllEvents();
		//for (auto it : allNetEvent)
		//{
		//	delete it.second;
		//}
		//allNetEvent.clear();

		//Timer::TimerMap& allTimer = m_pTimer->GetAllTimer();
		//for (auto it : allTimer)
		//{
		//	delete it.second;
		//}
		//allTimer.clear();
	}


	int EventCentre::CalculateTimeoutMs()
	{
		if(!m_activeEvs.empty())
			return 0;

		int timeoutMs = m_pTimer->GetNextTimeout();

		return timeoutMs;
	}


	void EventCentre::MakeWaittingToActive()
	{
		MutexGuard lock(m_mutex);

		if (!m_waittingEvs.empty())
		{
			m_activeEvs.insert(m_activeEvs.begin(), m_waittingEvs.begin(), m_waittingEvs.end());
			m_waittingEvs.clear();
		}
	}


	int EventCentre::RegisterEvent(const EventSharedPtr& pEvent)
	{

		if (!pEvent || pEvent->GetCentre())
			return -1;

		uint32 ev = pEvent->GetEv();

		const EventKey& evKey = pEvent->GetEvKey();

		int ret = 0;

		MutexGuard lock(m_mutex);

		if (ev & IO_CARE_EVENT /*&& !(ev & ~IO_CARE_EVENT)*/ && m_pPoller)
		{
			ret = m_pPoller->AddEvent(pEvent);
		}
		else if (ev & TIMEOUT_CART_EVENT /*&& !(ev & ~TIMEOUT_CART_EVENT)*/ && m_pTimer)
		{
			ret = m_pTimer->AddTimer(pEvent);
		}
		else
		{
			return -1;
		}

		if (0 != ret)
		{
			return ret;
		}

		pEvent->SetCenter(this);

		if(pEvent->m_registercb)
			pEvent->m_registercb();

		++m_eventsNum;

		return ret;
	}


	void EventCentre::CancelEvent(Event* pEvent)
	{
		MutexGuard lock(m_mutex);

		if (!pEvent || pEvent->GetCentre() != this)
			return;

		PushEvent(pEvent, EV_CANCEL);
		pEvent->SetCenter(NULL);
	}


	int EventCentre::DeleteEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		uint32 ev = pEvent->GetEv();

		const EventKey& evKey = pEvent->GetEvKey();

		MutexGuard lock(m_mutex);

		if (ev & IO_CARE_EVENT && m_pPoller)
		{
			m_pPoller->DelEvent(pEvent);
		}
		else if (ev & TIMEOUT_CART_EVENT && m_pTimer)
		{
			m_pTimer->DelTimer((TimerEvent*)pEvent);
		}
		else
		{
			return -1;
		}

		--m_eventsNum;

		return 0;
	}


	void EventCentre::PushEvent(Event* pEvent, short ev)
	{
		MutexGuard lock(m_mutex);

		if (!pEvent || pEvent->GetCentre() != this || !(pEvent->GetEv() & ev))
		{
			printf("push event failed!\n");
			return;
		}

		pEvent->PushCurEv(ev);

		m_waittingEvs.push_back(pEvent);

		m_waittintEvsCond.CondBroadCast();
	}


	void EventCentre::UpdateEvent(Event* pEvent, short op, short ev)
	{
		MutexGuard lock(m_mutex);

		if (!pEvent)
			return;
		
		const EventKey& key = pEvent->GetEvKey();

		if (m_pPoller->GetEvent(key.fd))
			m_pPoller->UpdateFd(key.fd, op, ev);
	}

	
	void EventCentre::WakeUp()
	{
		if (!m_wakeUpEvent->WakeUp())
		{
			printf("wake up falied! errno:%d\n", GetLastErrorno());
		}
	}

}



//EventCentrePool Implemet
namespace chaos
{
	EventCentrePool::EventCentrePool(int threadNum /*= 0*/) :
		m_threadNum(threadNum),
		m_started(false),
		m_pool(new ThreadPool(m_threadNum)),
		m_cond(m_mutex),
		m_startedCb(NULL)
	{	
	}


	EventCentrePool::~EventCentrePool()
	{
		m_pool->Stop();
	}


	void EventCentrePool::Start(const StartedCallBack& cb/* = NULL*/)
	{
		if (m_started)
			return;

		m_pool->Run();

		for (int i = 0; i < m_threadNum; ++i)
			m_pool->PushTask(std::bind(&EventCentrePool::Work, this));

		if (cb)
			m_startedCb = cb;
		else
		{
			MutexGuard lock(m_mutex);
			m_cond.CondWait();
		}
	}


	void EventCentrePool::Stop()
	{
		MutexGuard lock(m_mutex);
		for (auto centre : m_centres)
		{
			assert(centre);
			centre->Exit();
		}

		m_started = false;
	}


	int EventCentrePool::RegisterEvent(const EventSharedPtr& pEvent)
	{
		if (!pEvent || pEvent->GetCentre() || !m_started)
			return -1;

		EventCentre* pCentre = NULL;
		{
			MutexGuard lock(m_mutex);

			int eventNum = 0;
			for (auto centre : m_centres)
			{
				if (!pCentre)
					pCentre = centre;

				if (centre->GetEventsNum() < pCentre->GetEventsNum())
					pCentre = centre;
			}
		}

		if (!pCentre)
			return -1;

		return pCentre->RegisterEvent(pEvent);
	}


	void EventCentrePool::Work()
	{
		static std::atomic<int> worked(0);

		EventCentre centre;
		{	
			MutexGuard lock(m_mutex);
			m_centres.push_back(&centre);
		}

		if (++worked >= m_threadNum)
		{
			m_started = true;

			if (m_startedCb)
				m_startedCb(this);

			else
				m_cond.CondSignal();
		}

		centre.EventLoop();
	}
}



//Listener Implement
namespace chaos
{
	Listener::Listener(socket_t fd, const SockAddr& addr) :
		Event(EV_IOREAD, { fd }),
		m_socket(new Socket(fd)),
		m_cb(NULL)
#ifdef IOCP_ENABLE
		, m_acceptOls(new ACCEPT_OVERLAPPED[INIT_ASYNACCEPTING])
		, m_overlappedsRefCnt(new int(0))
		, m_acceptBuffers(new char* [INIT_ASYNACCEPTING])
#endif // IOCP_ENABLE

	{
		assert(m_socket);
		m_socket->CloseOnExec();

		memcpy(&m_addr, &addr, sizeof(SockAddr));

#ifdef IOCP_ENABLE
		assert(m_acceptOls);
		assert(m_acceptBuffers);

		for (int i = 0; i < INIT_ASYNACCEPTING; ++i)
		{
			memset(&m_acceptOls[i], 0, sizeof(ACCEPT_OVERLAPPED));

			m_acceptOls[i].inListenerPos = i;
			m_acceptOls[i].refcnt = m_overlappedsRefCnt;

			m_acceptBuffers[i] = new char[INIT_ACCEPTADDRBUF_SIZE] {0};

			//acceptex只使用一个数据块 用于存放连接后的地址信息
			//之后可通过GetAcceptExSockaddrs获得本地或远程的地址
			m_acceptOls[i].overlapped.wsabufs[0].buf = m_acceptBuffers[i];
			m_acceptOls[i].overlapped.wsabufs[0].len = INIT_ACCEPTADDRBUF_SIZE;

			m_acceptOls[i].acceptfd = INVALID_SOCKET;
			m_acceptOls[i].overlapped.fd = INVALID_SOCKET;

			m_acceptOls[i].overlapped.cb = std::bind(&Listener::AcceptComplete, this, std::placeholders::_1, std::placeholders::_2,
				std::placeholders::_3, std::placeholders::_4);
		}

#endif // IOCP_ENABLE

		SetRegisterCallback(std::move(std::bind(&Listener::RegisterCallback, this)));
	}


	Listener::~Listener()
	{
#ifdef IOCP_ENABLE
		//关闭还未连接的socket
		std::unordered_set<LPACCEPT_OVERLAPPED> oset;
		while (!m_acceptedq.empty())
		{
			oset.insert(m_acceptedq.front());
			m_acceptedq.pop();
		}

		for (int i = 0; i < INIT_ASYNACCEPTING; ++i)
		{
			if (oset.find(&m_acceptOls[i]) == oset.end() && m_acceptOls[i].acceptfd != INVALID_SOCKET)
			{
				Socket s(m_acceptOls[i].acceptfd);
				m_acceptOls[i].overlapped.eventDestroy = true;
			}
		}

		//每一个accept OVERLAPPED都和一个已创建待连接的socket绑定
		//在linsten socket或者已创建待连接的socket被close时,都会触发iocp并返回995(WSA_OPERATION_ABORTED)错误
		//所以对应的accept overlapped在iocp的回调中销毁,否则在iocp的回调中会访问已删除的overlapped
		if (m_acceptOls && 0 == m_overlappedsRefCnt)
		{
			delete[] m_acceptOls;
			delete m_overlappedsRefCnt;
		}

		if (m_acceptBuffers)
			delete[] m_acceptBuffers;
#endif // IOCP_ENABLE

		//delete socket必须放在最后.
		//close socket导致触发iocp时,须先让上面的IOCP_ENABLE代码中设置eventDestroy
		/*if (m_socket)
			delete m_socket;*/
	}


	std::shared_ptr<Listener> Listener::CreateListener(const char* ip /*= NULL*/, unsigned short port /*= 0*/,
		bool ipv6 /*= false*/, int opt /*= 0*/, const ListenerCb& cb/* = NULL*/)
	{
		int af = AF_INET;
		if (ipv6)
			af = AF_INET6;

		socket_t fd = socket(af, SOCK_STREAM, IPPROTO_TCP);
		if (-1 == fd)
		{
			printf("create socket failed!\n");
			return NULL;
		}

		int on = 1;
		if(opt & TCP_OPT_TCP_NODELAY)
		{
			if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&on, static_cast<socklen_t>(sizeof(on))))
			{
				printf("set tcp nodelay failed!\n");
				return NULL;
			}
		}

		//默认设置的选项
		if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, static_cast <socklen_t>(sizeof(on))))
			return NULL;

		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, static_cast <socklen_t>(sizeof(on))))
			return NULL;

		SockAddr sa;
		memset(&sa, 0, sizeof(sa));

		if (ipv6)
		{
			sa.sin6.sin6_family = af;
			sa.sin6.sin6_port = htons(port);
			if (ip)
			{
				if (inet_pton(af, ip, &sa.sin6.sin6_addr) <= 0)
				{
					printf("inet_pton in inet6 failed:%d\n", GetLastErrorno());
					return NULL;
				}
			}
			else
				sa.sin6.sin6_addr = in6addr_any;
		}
		else
		{
			sa.sin.sin_family = af;
			sa.sin.sin_port = htons(port);
			if (ip )
			{
				if (inet_pton(af, ip, &sa.sin.sin_addr) <= 0)
				{
					printf("inet_pton in inet4 failed:%d\n", GetLastErrorno());
					return NULL;
				}
			}
		}

		std::shared_ptr<Listener> listener(new Listener(fd, sa));

		listener->SetListenerCb(cb);

		return listener;
	}


	int Listener::Listen()
	{
		Socket& s = GetSocket();

		bool isListen = false;
		socklen_t optsize = sizeof(isListen);
		if (getsockopt(s.GetFd(), SOL_SOCKET, SO_ACCEPTCONN, (char*)&isListen, &optsize) || isListen)
			return -1;

		int ret = 0;

		//设置socket为非阻塞
		if (0 > (ret = s.SetNonBlock()))
			return ret;

		if (0 != (ret = s.Bind(&m_addr.sa, sizeof(m_addr))))
			return ret;

		if (0 != (ret = s.Listen(128)))
			return ret;

#ifdef IOCP_ENABLE
		if(0 != (ret = StartAsynAccept()))
			return ret;
#endif // IOCP_ENABLE

		return ret;
	}


	void Listener::Handle()
	{
		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{
			EventCentre* pCentre = GetCentre();
			if (!pCentre)
				return;

#ifdef IOCP_ENABLE
			if (m_acceptedq.empty())
				return;

			LPACCEPT_OVERLAPPED lo = m_acceptedq.front();
			m_acceptedq.pop();
			if (!lo)
				return;

			//投递新的accept事件
			AsynAccept(lo);
#else
			Socket& s = GetSocket();
			while (1)
			{
				socket_t acceptedfd = s.Accept();
#ifdef _WIN32
				//win的select调用
				if (INVALID_SOCKET == acceptedfd)
					break;
#else
				if (0 > acceptedfd)
				{
					if (errno != EAGAIN)
						printf("accept failed. errno:%d\n", errno);
					break;
				}
#endif // _WIN32
				DoneAccept(acceptedfd);
			} 
#endif // IOCP_ENABLE
		}
	}


	void Listener::DoneAccept(socket_t acceptedfd)
	{
		//Connecter* newconn = new Connecter(acceptedfd);

		//EventCentre* pCentre = GetCentre();
		//if (!pCentre)
		//	return;

		//int ret = pCentre->RegisterEvent(newconn);

		//if (0 != ret)
		//{
		//	delete newconn;
		//	return;
		//}

		//CallListenerCb(newconn);
		CallListenerCb(acceptedfd);
	}


	void Listener::RegisterCallback()
	{
		int ret = 0;
		if (0 != (ret = Listen()))
		{
			CallErr(ret);
			return;
		}

		printf("listen socket:%lld\n", GetSocket().GetFd());
	}


#ifdef IOCP_ENABLE
	int Listener::StartAsynAccept()
	{
		if (!m_acceptOls)
			return -1;

		int ret = 0;
		for (int i = 0; i < INIT_ASYNACCEPTING; ++i)
		{
			ret = AsynAccept(&m_acceptOls[i]);
			if (0 != ret)
			{
				CallErr(-1);
				return ret;
			}
		}

		return 0;
	}


	int Listener::AsynAccept(LPACCEPT_OVERLAPPED lo)
	{
		if (!lo)
			return -1;

		Socket& s = GetSocket();
		socket_t listenfd = s.GetFd();

		SockAddr addr;
		int addrlen = sizeof(addr);
		memset(&addr, 0, addrlen);
		if (getsockname(listenfd, (sockaddr*)&addr, &addrlen))
		{
			printf("getsocketname failed:%d\n", GetLastErrorno());
			return GetLastErrorno();
		}

		int type = 0;
		int typelen = sizeof(type);
		if (getsockopt(listenfd, SOL_SOCKET, SO_TYPE, (char*)&type, &typelen))
		{
			printf("getsockopt failed:%d\n", GetLastErrorno());
			return GetLastErrorno();
		}

		socket_t acceptfd = socket(addr.sa.sa_family, type, 0);

		lo->acceptfd = acceptfd;
		lo->overlapped.fd = listenfd;
		DWORD pending = 0;

		if (IOCP::AcceptEx(listenfd, lo->acceptfd, lo->overlapped.wsabufs[0].buf, 0, lo->overlapped.wsabufs[0].len / 2,
			lo->overlapped.wsabufs[0].len / 2, &pending, &lo->overlapped.overlapped))
		{
			++(*lo->refcnt);
			AcceptComplete(&lo->overlapped.overlapped, 0, 1, true);
		}
		else
		{
			int err = GetLastErrorno();
			if (ERROR_IO_PENDING != err)
			{
				printf("AcceptEx failed err:%d\n", err);
				return err;
			}
			else
				++(*lo->refcnt);
		}

		return 0;
	}


	void Listener::AcceptComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk)
	{
		if (!o)
		{
			CallErr(-1);
			return;
		}

		LPACCEPT_OVERLAPPED lo = (LPACCEPT_OVERLAPPED)o;
		--(*lo->refcnt);

		if (lo->overlapped.eventDestroy)
		{
			if (0 == (*lo->refcnt))
			{
				delete lo->refcnt;
				delete[](lo - lo->inListenerPos);		//delete只能从申请内存的起始位置开始
			}

			return;
		}

		if (!bOk)
			return;

		socket_t acceptedfd = lo->acceptfd;		//已经连接成功的fd
		socket_t listenfd = GetSocket().GetFd();

		if (0 != setsockopt(lo->acceptfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
			(char*)&listenfd, sizeof(listenfd)))
		{
			printf("setsockopt failed:%d\n", GetLastErrorno());
			return;
		}

		//准备投递下一次AcceptEx
		m_acceptedq.push(lo);
		PushEventToCentre(EV_IOREAD);

		DoneAccept(acceptedfd);

	}
#endif // IOCP_ENABLE
}



//Connecter Implement
namespace chaos
{
	Connecter::Connecter(socket_t fd) :
		Event(EV_IOREAD | EV_IOWRITE, { fd }),
		m_socket(new Socket(fd)),
		m_pReadBuffer(new Buffer),
		m_pWriteBuffer(new Buffer),
#ifdef IOCP_ENABLE
		m_pReadOverlapped(new COMPLETION_OVERLAPPED),
		m_isPostRecv(false),
		m_pWriteOverlapped(new COMPLETION_OVERLAPPED),
		m_isPostWrite(false),
		m_pConnectOverlapped(new COMPLETION_OVERLAPPED),
		m_isPostConnect(false),
#endif // IOCP_ENABLE
		m_readcb(NULL),
		m_writecb(NULL),
		m_connectcb(NULL),
		m_peeraddrlen(sizeof(SockAddr)),
		m_connected(false)
	{
		assert(m_socket);
		m_socket->CloseOnExec();

#ifdef IOCP_ENABLE
		assert(m_pReadOverlapped);
		assert(m_pWriteOverlapped);
		assert(m_pConnectOverlapped);

		memset(m_pReadOverlapped, 0, sizeof(COMPLETION_OVERLAPPED));
		m_pReadOverlapped->cb = std::bind(&Connecter::ReadComplete, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4);


		memset(m_pWriteOverlapped, 0, sizeof(COMPLETION_OVERLAPPED));
		m_pWriteOverlapped->cb = std::bind(&Connecter::WriteComplete, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4);

		memset(m_pConnectOverlapped, 0, sizeof(COMPLETION_OVERLAPPED));
		m_pConnectOverlapped->cb = std::bind(&Connecter::ConnectComplete, this, std::placeholders::_1, std::placeholders::_2,
			std::placeholders::_3, std::placeholders::_4);

#endif // IOCP_ENABLE

		SetRegisterCallback(std::move(std::bind(&Connecter::RegisterCallback, this)));

		memset(&m_peeraddr, 0, sizeof(m_peeraddr));

		getpeername(fd, &m_peeraddr.sa, &m_peeraddrlen);
		if (m_peeraddr.sa.sa_family != AF_UNSPEC)
			m_connected = true;
	}


	Connecter::~Connecter()
	{
		if (m_pReadBuffer)
			delete m_pReadBuffer;

		if (m_pWriteBuffer)
			delete m_pWriteBuffer;

#ifdef IOCP_ENABLE
		if (m_pReadOverlapped)
		{
			m_pReadOverlapped->eventDestroy = true;

			//在iocp的回调中通过eventDestroy的判定来释放;
			if (!m_isPostRecv)
			{
				delete m_pReadOverlapped;
			}
		}

		if (m_pWriteOverlapped)
		{
			m_pWriteOverlapped->eventDestroy = true;

			//在iocp的回调中通过eventDestroy的判定来释放;
			if (!m_isPostWrite)
			{
				delete m_pWriteOverlapped;
			}
		}

		if (m_pConnectOverlapped)
		{
			m_pConnectOverlapped->eventDestroy = true;

			//在iocp的回调中通过eventDestroy的判定来释放;
			if (!m_isPostConnect)
				delete m_pConnectOverlapped;
		}
#endif // IOCP_ENABLE

		//if (m_socket)
		//	delete m_socket;
	}


	void Connecter::Handle()
	{
		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{
#ifdef IOCP_ENABLE
			AsynRead();
#else
			HandleRead();
#endif // IOCP_ENABLE
		}

		if (ev & EV_IOWRITE)
		{
#ifdef IOCP_ENABLE
			AsynWrite();
#else
			HandleWrite();
#endif // IOCP_ENABLE
		}
	}


	int Connecter::Connect(sockaddr* sa, int salen)
	{
		if (!sa)
			return -1;

		int ret = m_socket->Connect(sa, salen);
		
		bool sucess = true;
		if (ret != 0 && errno != EISCONN /*&& SOCKET_ERR_NOT_TRY_AGAIN(errno)*/)
			sucess = false;

		getpeername(m_socket->GetFd(), &m_peeraddr.sa, &m_peeraddrlen);

		if (sucess)
		{
			m_connected = true;
			EnableEvent(IO_CARE_EVENT);
		}

		CallbackConnect(sucess);

		return ret;
//#endif // !IOCP_ENABLE

	}


#ifdef IOCP_ENABLE
	int Connecter::AsyncConnect(sockaddr* sa, int salen)
	{
		if (!sa)
			return -1;

		if (m_isPostConnect || m_connected)
			return 0;

		sockaddr_in ss;
		memset(&ss, 0, sizeof(ss));

		ss.sin_family = sa->sa_family;

		if (bind(m_socket->GetFd(), (struct sockaddr*) & ss, sizeof(ss)) < 0 &&
			WSAGetLastError() != WSAEINVAL)
		{
			printf("bind failed! err:%d\n", GetLastErrorno());
			return -1;
		}

		m_pConnectOverlapped->fd = m_socket->GetFd();

		int ret = IOCP::ConnectEx(m_socket->GetFd(), sa, salen, NULL, 0, NULL, &m_pConnectOverlapped->overlapped);
		if (ret)
		{
			if (GetLastErrorno() != WSA_IO_PENDING && errno != EISCONN/* && SOCKET_ERR_NOT_TRY_AGAIN(errno)*/)
			{
				printf("ConnectEx failed:%d\n", GetLastErrorno());
				ConnectComplete(&m_pConnectOverlapped->overlapped, ret, 0, false);
				return ret;
			}
			else
				m_isPostConnect = true;
		}

		return 0;
	}
#endif // IOCP_ENABLE


	int Connecter::WriteBuffer(const char* buf, int size)
	{
		uint32 written = m_pWriteBuffer->WriteBuffer(buf, size);

		if (0 >= written)
			return -1;

		EventCentre* pCentre = GetCentre();

		if (!pCentre)
			return -1;

		EnableEvent(EV_IOWRITE);

		return written;
	}


	int Connecter::Send(const char* buf, int size)
	{
		Socket& s = GetSocket();

		int written = 0;
		while (written < size)
		{
			int writelen = s.Send(buf + written, size - written);
			if (0 >= writelen)
			{
				if (SOCKET_ERR_NOT_TRY_AGAIN(errno))
					return -1;
				else
					break;
			}

			written += writelen;
		}

		return written;
	}


	int Connecter::ReadBuffer(char* buf, int size)
	{
		int readed = m_pReadBuffer->ReadBuffer(buf, size);

		return readed;
	}


	void Connecter::EnableEvent(short ev)
	{
		short valid = 0;


		if (ev & EV_IOREAD)
			valid |= EV_IOREAD;
		if (ev & EV_IOWRITE)
			valid |= EV_IOWRITE;
		if (ev & EV_IOEXCEPT)
			valid |= EV_IOEXCEPT;

		UpdateEvent(EV_CTL_ADD, valid);

#ifdef IOCP_ENABLE
		if (ev & EV_IOREAD && !m_isPostRecv && m_connected)
			AsynRead();

		if (ev & EV_IOWRITE && !m_isPostWrite && m_connected)
			AsynWrite();
#endif // IOCP_ENABLE
	}


	void Connecter::DisableEvent(short ev)
	{
		short valid = 0;

		if (ev & EV_IOREAD)
			valid |= EV_IOREAD;
		if (ev & EV_IOWRITE)
			valid |= EV_IOWRITE;
		if (ev & EV_IOEXCEPT)
			valid |= EV_IOEXCEPT;

		UpdateEvent(EV_CTL_DEL, valid);
	}


	void Connecter::RegisterCallback()
	{
#ifdef IOCP_ENABLE
		//(!m_readcb && readcb)只在第一次有效的设置回调时投递异步事件
		//用m_peeraddr判断Connecter是否已连接
		//如果未连接就投递异步事件会导致iocp返回错误而删除该事件
		if (!m_isPostRecv && m_connected)
			AsynRead();

		if (!m_isPostWrite && m_connected)
			AsynWrite();
#endif // IOCP_ENABLE
	}


	int Connecter::HandleRead()
	{
		Socket& s = GetSocket();

		int unread = s.GetUnreadByte();
		if (unread <= 0)
			unread = MAX_SINGLE_READ_DEFAULT;

		IOVEC_TYPE	iovec[MAX_IOVEC];
		memset(iovec, 0, sizeof(iovec));

		int iovcnt = m_pReadBuffer->GetWriteBuffer(iovec, MAX_IOVEC, unread);

		int readBytes = s.Recv(iovec, iovcnt);

		if (readBytes < 0 && SOCKET_ERR_NOT_TRY_AGAIN(errno))
		{
			CallErr(GetLastErrorno());
			return readBytes;
		}

		if (0 == readBytes)
		{
			CallErr(0);
			return 0;
		}

		if(readBytes > 0)
			m_pReadBuffer->MoveWritePos(readBytes);

		if (unread - readBytes > 0 )
			EnableEvent(EV_IOREAD);

		CallbackRead(readBytes);

		return 0;
	}


	int Connecter::HandleWrite()
	{
		Socket& s = GetSocket();

		uint32 readable = m_pWriteBuffer->GetReadSize();
		if (readable <= 0)
			return 0;

		int sendBytes = 0;

		IOVEC_TYPE	iovec[MAX_IOVEC];
		memset(iovec, 0, sizeof(iovec));

		int iovcnt = m_pWriteBuffer->ReadBuffer(iovec, MAX_IOVEC, readable);

		sendBytes = s.Send(iovec, iovcnt);
		if (sendBytes < 0 && SOCKET_ERR_NOT_TRY_AGAIN(errno))
		{
			CallErr(sendBytes);
			return sendBytes;
		}

		m_pWriteBuffer->MoveReadPos(sendBytes);

		if (m_pWriteBuffer->GetReadSize() <= 0)
			DisableEvent(EV_IOWRITE);
		else
			EnableEvent(EV_IOWRITE);

		CallbackWrite(sendBytes);

		return 0;
	}


#ifdef IOCP_ENABLE
	int Connecter::AsynRead()
	{
		if (m_isPostRecv)
		{
			printf("is in post!\n");
			return 0;
		}

		Socket& s = GetSocket();

		int iovcnt = m_pReadBuffer->GetWriteBuffer(m_pReadOverlapped->wsabufs, MAX_IOVEC, MAX_SINGLE_READ_DEFAULT);

		m_pReadOverlapped->fd = s.GetFd();

		DWORD readBytes = 0;
		DWORD flags = 0;

		if (!(GetEv() & EV_IOREAD))
		{
			printf("not register io_read!\n");
			return -1;
		}

		int ret = WSARecv(m_pReadOverlapped->fd, m_pReadOverlapped->wsabufs, iovcnt, &readBytes, &flags, &m_pReadOverlapped->overlapped, NULL);
		if (ret)
		{
			if (GetLastErrorno() != WSA_IO_PENDING)
			{
				printf("WSARecv failed:%d\n", GetLastErrorno());
				ReadComplete(&m_pReadOverlapped->overlapped, ret, 0, false);
				return ret;
			}
		}
		//立即返回也会触发 GetQueuedCompletionStatus 所以这里不需要手动调用callback;
		//else
		//{
		//	ReadComplete(&m_pReadOverlapped->overlapped, bytesRead, 0, true);
		//}

		m_isPostRecv = true;

		return 0;
	}


	int Connecter::AsynWrite()
	{
		if (m_isPostWrite)
			return 0;

		uint32 readable = m_pWriteBuffer->GetReadSize();
		if (readable <= 0)
			return 0;
		
		if (readable > MAX_SINGLE_WRITE_DEFAULT)
			readable = MAX_SINGLE_WRITE_DEFAULT;

		int iovcnt = m_pWriteBuffer->ReadBuffer(m_pWriteOverlapped->wsabufs, MAX_IOVEC, readable);

		DWORD sendBytes = 0;
		DWORD flags = 0;

		if (!(GetEv() & EV_IOWRITE))
			return -1;

		int ret = WSASend(GetSocket().GetFd(), m_pWriteOverlapped->wsabufs, iovcnt, &sendBytes, flags, &m_pWriteOverlapped->overlapped, NULL);
		if (ret)
		{
			if (GetLastErrorno() != WSA_IO_PENDING)
			{
				printf("WSASend failed:%d\n", GetLastErrorno());
				WriteComplete(&m_pWriteOverlapped->overlapped, sendBytes, 0, false);
				return GetLastErrorno();
			}
		}
		//立即返回也会触发 GetQueuedCompletionStatus 所以这里不需要手动调用callback;
		//else
		//{
		//	WriteComplete(&m_pWriteOverlapped->overlapped, sendBytes, 0, true);
		//}

		m_isPostWrite = true;

		return 0;
	}


	void Connecter::ReadComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk)
	{
		assert(o);

		LPCOMPLETION_OVERLAPPED lo = (LPCOMPLETION_OVERLAPPED)o;

		if (lo->eventDestroy)
		{
			delete lo;
			return;
		}

		m_isPostRecv = false;

		if (!bOk)
		{
			int n = WSAGetLastError();
			CallErr(-1);
			return;
		}

		//收完数据后调整buffer下次写入的位置
		m_pReadBuffer->MoveWritePos(bytes);

		//准备投递下一次WSARecv
		if(0 < bytes)
			PushEventToCentre(EV_IOREAD);

		CallbackRead(bytes);
	}


	void Connecter::WriteComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk)
	{
		assert(o);

		LPCOMPLETION_OVERLAPPED lo = (LPCOMPLETION_OVERLAPPED)o;

		if (lo->eventDestroy)
		{
			delete lo;
			return;
		}

		m_isPostWrite = false;

		if (!bOk)
		{
			int n = WSAGetLastError();
			CallErr(-1);
			return;
		}

		m_pWriteBuffer->MoveReadPos(bytes);

		int readable = m_pWriteBuffer->GetReadSize();

		if (readable > 0)
			PushEventToCentre(EV_IOWRITE);

		CallbackWrite(bytes);
	}


	void Connecter::ConnectComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk)
	{
		assert(o);

		LPCOMPLETION_OVERLAPPED lo = (LPCOMPLETION_OVERLAPPED)o;

		if (lo->eventDestroy)
		{
			delete lo;
			return;
		}

		m_isPostConnect = false;

		setsockopt(lo->fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
		getpeername(lo->fd, &m_peeraddr.sa, &m_peeraddrlen);

		if (bOk)
		{
			m_connected = true;
			EnableEvent(IO_CARE_EVENT);
		}

		CallbackConnect(bOk);

	}

#endif // IOCP_ENABLE

}



//TimerEvent Implemet
namespace chaos
{
	TimerEvent::TimerEvent(uint32 timeoutMs, const TimerHandler& func /* = NULL*/, bool isLoop/* = false*/) :
		Event(EV_TIMEOUT, { Timer::CreateTimerID() }),
		m_timeoutMs(timeoutMs),
		m_isLoop(isLoop),
		m_isSuspend(false),
		m_handleFunc(func)
	{
		m_nextTime = GetCurrentMSec() + m_timeoutMs;
	}


	TimerEvent::~TimerEvent()
	{
		printf("delete timerevent!\n");
		Timer::ReleaseTimerID(GetEvKey().timerId);
	}


	void TimerEvent::Handle()
	{
		if (m_handleFunc)
			m_handleFunc(*this);
	}

}