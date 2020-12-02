#include "Event.h"
#include "Timer.h"
#include <stdio.h>
#include <stdexcept>


namespace chaos
{
	Event::Event(short ev, const EventKey& evKey) :
		m_pCenter(NULL),
		m_ev(ev | EV_CANCEL),
		m_userdata(NULL)
	{
		memcpy(&m_evKey, &evKey, sizeof(EventKey));
	}


	void Event::CancelEvent()
	{
		if (m_pCenter)
		{
			m_pCenter->PushEvent(this, EV_CANCEL);
			SetCenter(NULL);
		}
	}


	void Event::CallErr(int errcode)
	{
		if (m_callback)
			m_callback(this, GetCurEv() | EV_ERROR, m_userdata);
		else
			CancelEvent();
	}


	void Event::UpdateEvent(short op, short ev)
	{
		if (!m_pCenter)
			return;

		if (EV_CTL_ADD == op)
			m_ev |= ev;
		else if (EV_CTL_DEL == op)
			m_ev &= ~ev;

		m_pCenter->UpdateEvent(this, op, ev);
	}

}	//namespace chaos



//EventCentre Implement
namespace chaos
{
	EventCentre::EventCentre() :
		m_pPoller(0),
		m_pTimer(0),
		m_running(false),
		m_evcount(0),
		m_isInit(false)
#ifndef _WIN32
		, m_mutex(PTHREAD_MUTEX_RECURSIVE)
#endif // !_WIN32
	{
	}


	EventCentre::~EventCentre()
	{
		ClearAllEvent();

		if (m_pPoller)
			delete m_pPoller;

		if (m_pTimer)
			delete m_pTimer;
	}


	int EventCentre::Init()
	{
		m_pPoller = Poller::AdapterNetDrive(this);
		if (!m_pPoller)
			return -1;

		m_pPoller->Init();

		m_pTimer = new Timer();
		if (!m_pTimer)
			return -1;

		m_isInit = true;

		return 0;
	}


	int EventCentre::EventLoop(int loopTickTimeMs/* = 0*/)
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

		if (0 >= loopTickTimeMs)
			loopTickTimeMs = 0;

		m_running = true;

		while (m_running)
		{
			//没有任何事件的时候不做后续处理(直接退出还是continue?)
			/*if (0 >= m_evcount)
				continue;*/

			if (!m_waittingEvs.empty())
			{
				m_activeEvs.insert(m_activeEvs.begin(), m_waittingEvs.begin(), m_waittingEvs.end());
				m_waittingEvs.clear();
			}

			int timeout = CalculateTimeout();

			if (0 != m_pPoller->Launch(timeout, m_activeEvs))
			{
				printf("called poller failed!\n");
				break;
			}

			/*if (0 != SignalDispatch())
				break;*/

			m_pTimer->DispatchTimer(m_activeEvs);

			if (0 != ProcessActiveEvent())
			{
				printf("process active event failed!\n");
				break;
			}
		}

		m_running = false;

		ClearAllEvent();

		printf("EventCentre exit!\n");

		return 0;
	}


	int EventCentre::ProcessActiveEvent()
	{
		if (m_activeEvs.empty())
			return 0;

		MutexGuard lock(m_mutex);

		for(auto pev : m_activeEvs)
		{
			//Event* pev = m_activeEvs.front();
			//m_activeEvs.pop();

			if (!pev)
				continue;

			//EV_CANCEL时GetCentre已为NULL
			//优先处理Centre,这个判断必须在GetCentre之前
			if (pev->GetCurEv() & EV_CANCEL)
			{
				pev->Callback();
				CancelEvent(pev);
				continue;
			}

			if (!pev->GetCentre())
				continue;

			pev->Handle();

			pev->Callback();

			//清除此次已处理的事件
			pev->PopCurEv();
		}

		m_activeEvs.clear();

		return 0;
	}


	void EventCentre::ClearAllEvent()
	{
		MutexGuard lock(m_mutex);

		if (m_pPoller)
		{
			Poller::NetEventMap& allNetEvent = m_pPoller->GetAllEvents();
			for (auto it : allNetEvent)
			{
				delete it.second;
			}
			allNetEvent.clear();
		}

		if (m_pTimer)
		{
			Timer::TimerMap& allTimer = m_pTimer->GetAllTimer();
			for (auto it : allTimer)
			{
				delete it.second;
			}
			allTimer.clear();
		}
	}


	int EventCentre::CalculateTimeout()
	{
		if(!m_activeEvs.empty())
			return 0;

		int timeout = m_pTimer->GetNextTimeout();

		return timeout;
	}


	int EventCentre::RegisterEvent(Event* pEvent)
	{

		if (!pEvent || (pEvent->GetCentre() && this != pEvent->GetCentre()))
		{
			return -1;
		}

		uint32 ev = pEvent->GetEv();

		const EventKey& evKey = pEvent->GetEvKey();

		int ret = 0;

		MutexGuard lock(m_mutex);

		if (ev & IO_CARE_EVENT && !(ev & ~IO_CARE_EVENT) && m_pPoller)
		{
			ret = m_pPoller->AddEvent(pEvent);
		}
		else if (ev & TIMEOUT_CART_EVENT && !(ev & ~TIMEOUT_CART_EVENT) && m_pTimer)
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
		pEvent->CallbackRegister(ret);

		++m_evcount;

		return ret;
	}


	int EventCentre::CancelEvent(Event* pEvent)
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

		--m_evcount;

		delete pEvent;

		return 0;
	}


	void EventCentre::PushEvent(Event* pEvent, short ev)
	{
		MutexGuard lock(m_mutex);

		if (!pEvent || pEvent->GetCentre() != this || !(pEvent->GetEv() & ev))
			return;

		pEvent->PushCurEv(ev);

		m_waittingEvs.push_back(pEvent);
	}


	void EventCentre::UpdateEvent(Event* pEvent, short op, short ev)
	{
		if (!pEvent)
			return;

		const EventKey& key = pEvent->GetEvKey();

		if (m_pPoller->GetEvent(key.fd))
			m_pPoller->UpdateFd(key.fd, op, ev);
	}

}



//Listener Implement
namespace chaos
{
	Listener::Listener(socket_t fd) :
		Event(EV_IOREAD, (EventKey&)fd),
		m_socket(new Socket(fd)),
		m_cb(NULL),
		m_userdata(0)
#ifdef IOCP_ENABLE
		, m_acceptOls(0)
		, m_overlappedsRefCnt(0)
		, m_acceptBuffers(0)
#endif // IOCP_ENABLE

	{
#ifdef IOCP_ENABLE
		m_acceptOls = new ACCEPT_OVERLAPPED[INIT_ASYNACCEPTING];
		if (!m_acceptOls)
			return;

		m_overlappedsRefCnt = new int;
		*m_overlappedsRefCnt = 0;

		m_acceptBuffers = new char* [INIT_ASYNACCEPTING];
		if (!m_acceptBuffers)
			return;

		for (int i = 0; i < INIT_ASYNACCEPTING; ++i)
		{
			memset(&m_acceptOls[i], 0, sizeof(ACCEPT_OVERLAPPED));

			m_acceptOls[i].inListenerPos = i;
			m_acceptOls[i].refcnt = m_overlappedsRefCnt;

			//m_acceptBuffers[i].Reserver(INIT_ACCEPTADDRBUF_SIZE);
			if(!(m_acceptBuffers[i] = new char[INIT_ACCEPTADDRBUF_SIZE]))
				return;

			//acceptex只使用一个数据块 用于存放连接后的地址信息
			//之后可通过GetAcceptExSockaddrs获得本地或远程的地址
			//uint32 n = 0;
			m_acceptOls[i].overlapped.databufs[0].buf = m_acceptBuffers[i];// .GetWriteBuffer(&n);
			m_acceptOls[i].overlapped.databufs[0].len = INIT_ACCEPTADDRBUF_SIZE;

			m_acceptOls[i].acceptfd = INVALID_SOCKET;
			m_acceptOls[i].overlapped.fd = INVALID_SOCKET;

			m_acceptOls[i].overlapped.cb = std::bind(&Listener::AcceptComplete, this, std::placeholders::_1, std::placeholders::_2,
				std::placeholders::_3, std::placeholders::_4);
		}

		SetRegisterCallback(std::bind(&Listener::RegisterCallback, this, std::placeholders::_1));

#endif // IOCP_ENABLE
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
				m_acceptOls[i].overlapped.eventDestroy = 1;
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
		if (m_socket)
			delete m_socket;
	}


	Listener* Listener::CreateListener(int af, int socktype, int protocol, unsigned short port, 
		const char* ip /*= NULL*/, ListenerCb cb/* = NULL*/, void* userdata/* = NULL*/)
	{
		socket_t fd = socket(af, socktype, protocol);
		if (-1 == fd)
		{
			printf("create socket failed!\n");
			return NULL;
		}

		Listener* listener = new Listener(fd);
		if (!listener)
		{
			printf("allocate listener failed!\n");
			return NULL;
		}

		listener->SetListenerCb(cb, userdata);

		sockaddr_in sa;
		memset(&sa, 0, sizeof(sa));
		sa.sin_family = af;
		sa.sin_port = htons(port);
		if(ip)
			inet_pton(af, ip, &sa.sin_addr);

		if (0 != listener->Listen((sockaddr*)&sa, sizeof(sa)))
		{
			printf("listen failed! err:%d\n", GetLastErrorno());
			delete listener;
			return NULL;
		}

		printf("listen socket:%lld\n", fd);

		return listener;
	}


	int Listener::Listen(const sockaddr* sa, int salen)
	{
		Socket& s = GetSocket();

		int ret = 0;

		int on = 1;
		if (0 > (ret = setsockopt(s.GetFd(), SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on))))
			return ret;

		//设置socket为非阻塞
		if (0 > (ret = s.SetNonBlock()))
			return ret;

		if (0 != (ret = s.Bind(sa, salen)))
			return ret;

		if (0 != (ret = s.Listen(128)))
			return ret;

		//int flag = 1;
		//ret = setsockopt(s.GetFd(), IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
		//if (0 != ret)
		//{
		//	printf("set TCP_NODELAY failed\n");
		//	return ret;
		//}

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


	void Listener::RegisterCallback(int ret)
	{
		if (0 != ret)
			return;

#ifdef IOCP_ENABLE
		StartAsynRequest();
#endif // IOCP_ENABLE
	}



	void Listener::DoneAccept(socket_t acceptedfd)
	{
		Connecter* newconn = new Connecter(acceptedfd);
		if (!newconn)
			return;

		EventCentre* pCentre = GetCentre();
		if (!pCentre)
			return;

		int ret = pCentre->RegisterEvent(newconn);

		if (0 != ret)
		{
			delete newconn;
			return;
		}

		CallListenerCb(newconn);
	}


#ifdef IOCP_ENABLE
	int Listener::StartAsynRequest()
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

		sockaddr_in addr;
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

		socket_t acceptfd = socket(addr.sin_family, type, 0);

		lo->acceptfd = acceptfd;
		lo->overlapped.fd = listenfd;
		DWORD pending = 0;

		if (IOCP::AcceptEx(listenfd, lo->acceptfd, lo->overlapped.databufs[0].buf, 0, lo->overlapped.databufs[0].len / 2,
			lo->overlapped.databufs[0].len / 2, &pending, &lo->overlapped.overlapped))
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

		EventCentre* pCentre = GetCentre();
		if (!pCentre)
		{
			CallErr(-1);
			return;
		}

		//Socket& s = GetSocket();

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
		//pCentre->PushActiveEv(this, EV_IOREAD);
		pCentre->PushEvent(this, EV_IOREAD);

		DoneAccept(acceptedfd);

	}
#endif // IOCP_ENABLE
}



//Connecter Implement
namespace chaos
{

	Connecter::Connecter(socket_t fd) :
		Event(EV_IOREAD | EV_IOWRITE, (EventKey&)fd),
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
		m_userdata(NULL),
		m_peeraddrlen(sizeof(SockAddr))
	{
#ifdef IOCP_ENABLE
		if (m_pReadOverlapped)
		{
			memset(m_pReadOverlapped, 0, sizeof(COMPLETION_OVERLAPPED));

			m_pReadOverlapped->cb = std::bind(&Connecter::ReadComplete, this, std::placeholders::_1, std::placeholders::_2,
				std::placeholders::_3, std::placeholders::_4);
		}

		if (m_pWriteOverlapped)
		{
			memset(m_pWriteOverlapped, 0, sizeof(COMPLETION_OVERLAPPED));

			m_pWriteOverlapped->cb = std::bind(&Connecter::WriteComplete, this, std::placeholders::_1, std::placeholders::_2,
				std::placeholders::_3, std::placeholders::_4);
		}

		if (m_pConnectOverlapped)
		{
			memset(m_pConnectOverlapped, 0, sizeof(COMPLETION_OVERLAPPED));

			m_pConnectOverlapped->cb = std::bind(&Connecter::ConnectComplete, this, std::placeholders::_1, std::placeholders::_2,
				std::placeholders::_3, std::placeholders::_4);
		}

#endif // IOCP_ENABLE

		SetRegisterCallback(std::bind(&Connecter::RegisterCallback, this, std::placeholders::_1));

		memset(&m_peeraddr, 0, sizeof(m_peeraddr));

		getpeername(fd, &m_peeraddr.sa, &m_peeraddrlen);
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
			m_pReadOverlapped->eventDestroy = 1;

			//在iocp的回调中通过eventDestroy的判定来释放;
			if (!m_isPostRecv)
				delete m_pReadOverlapped;
		}

		if (m_pWriteOverlapped)
		{
			m_pWriteOverlapped->eventDestroy = 1;

			//在iocp的回调中通过eventDestroy的判定来释放;
			if (!m_isPostWrite)
				delete m_pWriteOverlapped;
		}

		if (m_pConnectOverlapped)
		{
			m_pConnectOverlapped->eventDestroy = 1;

			//在iocp的回调中通过eventDestroy的判定来释放;
			if (!m_isPostConnect)
				delete m_pConnectOverlapped;
		}
#endif // IOCP_ENABLE

		if (m_socket)
			delete m_socket;
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
		if (!sa || !m_socket)
			return -1;

#ifdef IOCP_ENABLE
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
			if (GetLastErrorno() != WSA_IO_PENDING)
			{
				printf("ConnectEx failed:%d\n", GetLastErrorno());
				ConnectComplete(&m_pReadOverlapped->overlapped, ret, 0, false);
				return ret;
			}
			else
				m_isPostConnect = true;
		}

		return 0;
#else
		int ret = m_socket->Connect(sa, salen);

		getpeername(m_socket->GetFd(), &m_peeraddr.sa, &m_peeraddrlen);

		CallbackConnect(ret == 0 ? true : false);

		return ret;
#endif // !IOCP_ENABLE

	}


	int Connecter::WriteBuffer(const char* buf, int size)
	{
		if (!m_pWriteBuffer)
			return -1;

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
		if (!m_pReadBuffer)
			return 0;

		int readed = m_pReadBuffer->ReadBuffer(buf, size);

		return readed;
	}


	void Connecter::SetCallback(const NetCallback& readcb, const NetCallback& writecb, const NetCallback& connectcb, void* userdata)
	{
		//只在第一次有效的设置回调时投递异步事件
#ifdef IOCP_ENABLE
		//这里判断peeraddr是因为:在Connecter未连接时(调用Connect成功之前)调用该函数,
		//会导致投递异步事件发生错误而删除该事件
		if (!m_readcb && readcb && !m_isPostRecv && m_peeraddr.sa.sa_family != AF_UNSPEC)
			AsynRead();

		if (!m_writecb && writecb && !m_isPostWrite && m_peeraddr.sa.sa_family != AF_UNSPEC)
			AsynWrite();
#endif // IOCP_ENABLE

		m_readcb = readcb;
		m_writecb = writecb;
		m_connectcb = connectcb;
		m_userdata = userdata;
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
		if (GetEv() & EV_IOREAD && !m_isPostRecv && m_peeraddr.sa.sa_family != AF_UNSPEC)
			AsynRead();

		if (GetEv() & EV_IOWRITE && !m_isPostWrite && m_peeraddr.sa.sa_family != AF_UNSPEC)
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


	int Connecter::HandleRead()
	{
		if (!m_pReadBuffer)
			return -1;

		Socket& s = GetSocket();

		socket_unread_t unread = s.GetUnreadByte();
		int transferBytes = 0;

		while (0 < unread)
		{
			uint32 size = 0;

			char* buf = m_pReadBuffer->GetWriteBuffer(&size);
			if (!buf)
				break;

			int read = s.Recv(buf, size);

			if (0 >= read && SOCKET_ERR_NOT_TRY_AGAIN(errno))
			{
				transferBytes = 0;
				break;
			}

			m_pReadBuffer->MoveWriteBufferPos(read);

			unread -= read;
			transferBytes += read;
		}

		if (0 == transferBytes)
			CancelEvent();

		CallbackRead(transferBytes);

		return 0;
	}


	int Connecter::HandleWrite()
	{
		if (!m_pWriteBuffer)
			return -1;

		Socket& s = GetSocket();

		uint32 size = m_pWriteBuffer->GetReadSize();
		int tranferBytes = 0;

		while (0 < size)
		{
			uint32 hasbytes = 0;
			char* buf = m_pWriteBuffer->ReadBuffer(&hasbytes);
			if (!buf)
				break;
			
			if (0 == hasbytes)
				break;

			int sendSize = 0;
			do
			{
				sendSize = s.Send(buf, hasbytes);
				if (0 >= sendSize)	// && UTIL_ERR_NOT_TRY_AGAIN(errno))
					break;

				size -= sendSize;
				tranferBytes += sendSize;
				hasbytes -= sendSize;
				m_pWriteBuffer->MoveReadBufferPos(sendSize);

			} while (hasbytes > 0);

			if (0 >= sendSize)
				break;
		}

		if (m_pWriteBuffer->GetReadSize() <= 0)
			DisableEvent(EV_IOWRITE);
		else
			EnableEvent(EV_IOWRITE);

		CallbackWrite(tranferBytes);

		return 0;
	}


	void Connecter::RegisterCallback(int ret)
	{
		if (0 != ret)
			return;

#ifdef IOCP_ENABLE
		//AsynRead();
		//AsynWrite();
#endif // IOCP_ENABLE

	}


#ifdef IOCP_ENABLE

	int Connecter::AsynRead()
	{
		if (!m_pReadOverlapped || !m_pReadBuffer)
		{
			CallErr(-1);
			return -1;
		}

		Socket& s = GetSocket();

		uint32 size = 0;
		m_pReadOverlapped->databufs[0].buf = m_pReadBuffer->GetWriteBuffer(&size);
		m_pReadOverlapped->databufs[0].len = size;

		m_pReadOverlapped->fd = s.GetFd();

		DWORD bytesRead = 0;
		DWORD flags = 0;

		if (!(GetEv() & EV_IOREAD))
			return -1;

		int ret = WSARecv(m_pReadOverlapped->fd, &m_pReadOverlapped->databufs[0], 1, &bytesRead, &flags, &m_pReadOverlapped->overlapped, NULL);
		if (ret)
		{
			if (GetLastErrorno() != WSA_IO_PENDING)
			{
				printf("WSARecv failed:%d\n", GetLastErrorno());
				ReadComplete(&m_pReadOverlapped->overlapped, ret, 0, false);
				return ret;
			}
			else
				m_isPostRecv = true;
		}
		//立即返回也会触发 GetQueuedCompletionStatus 所以这里不需要手动调用callback;
		//else
		//{
		//	ReadComplete(&m_pReadOverlapped->overlapped, bytesRead, 0, true);
		//}

		return 0;
	}


	int Connecter::AsynWrite()
	{
		if (!m_pWriteOverlapped || !m_pWriteBuffer)
		{
			CallErr(-1);
			return -1;
		}

	
		uint32 readySize = m_pWriteBuffer->GetReadSize();

		while (0 < readySize)
		{
			uint32 readSize = readySize;
			m_pWriteOverlapped->databufs[0].buf = m_pWriteBuffer->ReadBuffer(&readSize);
			m_pWriteOverlapped->databufs[0].len = readSize;

			if (!m_pWriteOverlapped->databufs[0].buf || 0 == readSize)
			{	
				printf("error read buffer!\n");
				break;
			}

			DWORD sendBytes = 0;
			DWORD flags = 0;

			if (!(GetEv() & EV_IOWRITE))
				return -1;

			int ret = WSASend(GetSocket().GetFd(), &m_pWriteOverlapped->databufs[0], 1, &sendBytes, flags, &m_pWriteOverlapped->overlapped, NULL);
			if (ret)
			{
				if (GetLastErrorno() != WSA_IO_PENDING)
				{
					printf("WSASend failed:%d\n", GetLastErrorno());
					WriteComplete(&m_pWriteOverlapped->overlapped, ret, 0, false);
					return  GetLastErrorno();
				}
				else
				{
					m_isPostWrite = true;
				}
			}
			//立即返回也会触发 GetQueuedCompletionStatus 所以这里不需要手动调用callback;
			//else
			//{
			//	WriteComplete(&m_pWriteOverlapped->overlapped, sendBytes, 0, true);
			//}
		}

		return 0;
	}


	void Connecter::ReadComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk)
	{
		LPCOMPLETION_OVERLAPPED lo = (LPCOMPLETION_OVERLAPPED)o;
		if (!lo)
		{
			printf("read overlapped is null\n");
			CallErr(-1);
			return;
		}

		if (lo->eventDestroy)
		{
			delete lo;
			return;
		}

		if (!bOk)
		{
			CallErr(-1);
			return;
		}

		m_isPostRecv = false;

		//收完数据后调整buffer下次写入的位置
		m_pReadBuffer->MoveWriteBufferPos(bytes);

		EventCentre* pCentre = GetCentre();
		if (!pCentre)
			return;

		//准备投递下一次WSARecv
		if(0 < bytes)
			pCentre->PushEvent(this, EV_IOREAD);

		CallbackRead(bytes);
	}


	void Connecter::WriteComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk)
	{
		LPCOMPLETION_OVERLAPPED lo = (LPCOMPLETION_OVERLAPPED)o;
		if (!lo)
		{
			printf("write overlapped is null\n");
			CallErr(-1);
			return;
		}

		if (lo->eventDestroy)
		{
			delete lo;
			return;
		}

		m_isPostWrite = false;

		CallbackWrite(bytes);
	}


	void Connecter::ConnectComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk)
	{
		LPCOMPLETION_OVERLAPPED lo = (LPCOMPLETION_OVERLAPPED)o;
		if (!lo)
		{
			printf("connect overlapped is null\n");
			CallErr(-1);
			return;
		}

		if (lo->eventDestroy)
		{
			delete lo;
			return;
		}

		m_isPostConnect = false;

		setsockopt(lo->fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
		getpeername(lo->fd, &m_peeraddr.sa, &m_peeraddrlen);

		//在调用Connect之前设置了回调 将在此生效
		if (m_readcb && !m_isPostRecv && GetEv() & EV_IOREAD && m_peeraddr.sa.sa_family != AF_UNSPEC)
			AsynRead();

		if (m_writecb && !m_isPostWrite && GetEv() & EV_IOWRITE && m_peeraddr.sa.sa_family != AF_UNSPEC)
			AsynWrite();

		CallbackConnect(bOk);

	}

#endif // IOCP_ENABLE

}



namespace chaos
{
	TimerEvent::TimerEvent(uint32 timeout, bool isLoop/* = false*/) :
		Event(EV_TIMEOUT, EventKey(Timer::CreateTimerID())),	//EventKey{ Timer::CreateTimerID() }
		m_timeout(timeout),
		m_isLoop(isLoop),
		m_isCancel(false),
		m_isSuspend(false),
		m_handleFunc(NULL)
	{
		m_nextTime = time(NULL) + m_timeout;
	}


	TimerEvent::~TimerEvent()
	{
	}


	void TimerEvent::Handle()
	{
		if (m_handleFunc)
			m_handleFunc();
		else
			DefaultHandle();
	}

	
	void TimerEvent::DefaultHandle()
	{
		printf("test default timer handle!\n");
	}
}
