#include "Event.h"
#include "Timer.h"
#include <stdio.h>



namespace chaos
{
	Event::Event(EventCentre* pCentre, short ev, const EventKey& evKey) :
		m_pCenter(pCentre),
		m_ev(ev),
		m_curEv(0)
	{
		memcpy(&m_evKey, &evKey, sizeof(EventKey));
	}


	Event::Event() :
		m_pCenter(NULL),
		m_ev(0),
		m_curEv(0)
	{
		memset(&m_evKey, 0, sizeof(m_evKey));
	}


	void Event::CancelEvent()
	{
		if (m_pCenter)
			m_pCenter->CancelEvent(this);
	}


	EventCentre::EventCentre() :
		m_pPoller(0),
		m_pTimer(0),
		m_running(false)
		//m_evcount(0)
	{

	}


	EventCentre::~EventCentre()
	{
		if (m_pPoller)
			delete m_pPoller;

		if (!m_pTimer)
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

		return 0;
	}


	int EventCentre::EventLoop()
	{
		m_running = true;

		while (m_running)
		{
			if (0 >= m_evcount)
				break;

			if (0 != Dispatch())
				break;
		}

		return 0;
	}


	int EventCentre::Dispatch()
	{
		int ret = 0;

		if (0 != (ret = NetEventDispatch()))
			return ret;

		if (0 != (ret = SignalDispatch()))
			return ret;

		if (0 != (ret = TimerDispatch()))
			return ret;

		if(0 != (ret = ProcessActiveEvent()))
			return ret;

		return ret;
	}


	int EventCentre::ProcessActiveEvent()
	{
		m_mutex.Lock();

		while (!m_activeEvs.empty())
		{
			Event* pev = m_activeEvs.front();
			if (!pev)
				continue;

			pev->Handle();

			pev->Callback();

			//清除此次已处理的事件
			pev->SetCurEv(0);

			m_activeEvs.pop();
		}

		m_mutex.UnLock();

		return 0;
	}


	int EventCentre::RegisterEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		uint32 ev = pEvent->GetEv();

		const EventKey& evKey = pEvent->GetEvKey();

		int ret = 0;

		if (ev & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT)
			&& !(ev & ~(EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
			&& m_pPoller)
		{
			ret = m_pPoller->AddEvent(pEvent);
		}
		else if (ev & EV_TIMEOUT && !(ev & ~EV_TIMEOUT) && m_pTimer)
		{
			ret = m_pTimer->AddTimer(pEvent);
		}
		else if (ev & EV_SIGNAL && !(ev & ~EV_SIGNAL))
			m_signalEvs.insert(std::make_pair(evKey.signal, pEvent));
		else
			return -1;
		
		if (0 != ret)
			return ret;

		++m_evcount;

		return ret;
	}


	int EventCentre::CancelEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		uint32 ev = pEvent->GetEv();

		const EventKey& evKey = pEvent->GetEvKey();

		if (ev & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT) && m_pPoller)
		{
			m_pPoller->DelEvent(pEvent);
		}
		else if (ev & EV_TIMEOUT && m_pTimer)
		{
			m_pTimer->DelTimer((TimerEvent*)pEvent);
		}
		else if (ev & EV_SIGNAL)
			m_signalEvs.erase(evKey.signal);
		else
			return -1;

		--m_evcount;

		return 0;
	}


	void EventCentre::PushActiveEv(Event* pEvent, short ev)
	{
		if (!pEvent)
			return;

		pEvent->SetCurEv(pEvent->GetCurEv() | ev);

		PushActiveEv(pEvent);
	}


	int EventCentre::NetEventDispatch()
	{
		if (!m_pPoller)
			return -1;

		int ret = 0;
		if (0 != (ret = m_pPoller->Launch()))
			return ret;

		return ret;
	}


	int EventCentre::SignalDispatch()
	{
		return 0;
	}


	int EventCentre::TimerDispatch()
	{
		if (!m_pTimer)
			return -1;

		m_pTimer->DispatchTimer();

		return 0;
	}

}



namespace chaos
{

	int Listener::Listen(const sockaddr* sa, int salen)
	{
		Socket& s = GetSocket();

		int ret = 0;

		//设置socket为非阻塞
		if (0 > (ret = s.SetNonBlock()))
			return ret;

		if (0 != (ret = s.Bind(sa, salen)))
			return ret;

		if (0 != (ret = s.Listen()))
			return ret;

#ifdef _WIN32
		//sockaddr_in addr;
		//int addrlen = sizeof(addr);
		//if (getsockname(s.GetFd(), (sockaddr*)&addr, &addrlen))
		//	return WSAGetLastError();
		
		if (!m_pAcceptOl)
			return -1;

		int type = 0;
		int typelen = sizeof(type);
		if (getsockopt(s.GetFd(), SOL_SOCKET, SO_TYPE, (char*)&type, &typelen))
			return WSAGetLastError();

		for (int i = 0; i < INIT_ASYNACCEPTING; ++i)
		{
			socket_t fd = socket(sa->sa_family, type, 0);
			m_pAcceptOl[i].acceptfd = fd;
			AsynAccept(&m_pAcceptOl[i]);
		}
#endif // _WIN32

		return ret;
	}


	void Listener::Handle()
	{
		Socket& s = GetSocket();
		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{
			EventCentre* pCentre = GetCentre();
			if (!pCentre)
				return;

#ifdef _WIN32
			//if (!m_pOverlapped)
			//	return;

			//NetEvent* pNewEv = new Connecter(pCentre, m_pOverlapped->acceptfd);
			//if (!pNewEv)
			//	return;

			//int ret = pCentre->RegisterEvent(pNewEv);
			//if (0 != ret)
			//	return;

			//CallListenerCb(pNewEv);

			////投递新的accept事件
			//sockaddr_in addr;
			//int addrlen = sizeof(addr);
			//if (getsockname(s.GetFd(), (sockaddr*)&addr, &addrlen))
			//	return;

			//int type = 0;
			//int typelen = sizeof(type);
			//if (getsockopt(s.GetFd(), SOL_SOCKET, SO_TYPE, (char*)&type, &typelen))
			//	return ;

			//socket_t fd = socket(addr.sin_family, type, 0);
			//AsynAccept(fd);
#else
			while (1)
			{
				socket_t connfd = s.Accept();
				if (0 > connfd || errno == EAGAIN || connfd == INVALID_SOCKET)
					break;

				NetEvent* pNewEv = NULL;
//#ifdef _WIN32
//				pNewEv = new AsynConnecter(pCentre, connfd);
//#else
//				pNewEv = new Connecter(pCentre, connfd);
//#endif
				pNewEv = new Connecter(pCentre, connfd);
				if (!pNewEv)
				{
					return;
				}

				int ret = pCentre->RegisterEvent(pNewEv);
				if (0 != ret)
					return;

				CallListenerCb(pNewEv);
			} 
#endif // _WIN32
		}

	}


#ifdef _WIN32
	int Listener::AsynAccept(LPACCEPT_OVERLAPPED_DATA lo)
	{
		if (!lo)
			return -1;

		Socket& s = GetSocket();

		socket_t listenfd = s.GetFd();

		if (0 != setsockopt(lo->acceptfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
			(char*)&listenfd, sizeof(listenfd)))
			return WSAGetLastError();
		
		if(!IOCP::AcceptEx)
			return -1;

		//lo->acceptfd = fd;
		lo->overlapped.fd = listenfd;
		DWORD pending = 0;

		if (IOCP::AcceptEx(listenfd, lo->acceptfd, lo->overlapped.databuf.buf, 0, lo->overlapped.databuf.len,
			lo->overlapped.databuf.len, &pending, &lo->overlapped.overlapped))
		{
			//EventCentre* pCentre = GetCentre();
			//if (!pCentre)
			//	return -1;

			//NetEvent* pNetEvent = new Connecter(pCentre, lo->acceptfd);
			//if (!pNetEvent)
			//	return -1;

			//int ret = pCentre->RegisterEvent(pNetEvent);
			//if (0 != ret)
			//	return ret;

			//CallListenerCb(pNetEvent);

			IocpCallback(&lo->overlapped.overlapped, true);
		}
		else
		{
			int err = WSAGetLastError();
			if (ERROR_IO_PENDING != err)
				return err;
		}

		return 0;
	}


	void Listener::IocpCallback(OVERLAPPED* o, bool sucess)
	{
		if (!sucess || !o)
			return;

		EventCentre* pCentre = GetCentre();
		if (!pCentre)
			return;

		//pCentre->PushActiveEv(this, EV_IOREAD);

		Socket& s = GetSocket();

		sockaddr_in addr;
		int addrlen = sizeof(addr);
		if (getsockname(s.GetFd(), (sockaddr*)&addr, &addrlen))
			return;

		int type = 0;
		int typelen = sizeof(type);
		if (getsockopt(s.GetFd(), SOL_SOCKET, SO_TYPE, (char*)&type, &typelen))
			return ;

		LPACCEPT_OVERLAPPED_DATA lo = (LPACCEPT_OVERLAPPED_DATA)o;
		socket_t acceptedfd = lo->acceptfd;		//已经连接成功的fd

		NetEvent* pNewEv = new Connecter(pCentre, acceptedfd);
		if (!pNewEv)
			return;

		int ret = pCentre->RegisterEvent(pNewEv);
		if (0 != ret)
			return;

		//对该次连接成功的fd投递WSARecv
		pCentre->PushActiveEv(pNewEv, EV_IOREAD);

		CallListenerCb(pNewEv);

		socket_t fd = socket(addr.sin_family, type, 0);
		lo->acceptfd = fd;

		AsynAccept(lo);

	}
#endif // _WIN32

}


namespace chaos
{

	void Connecter::Handle()
	{
		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{
#ifdef _WIN32

#else
			HandleRead();
#endif // WIN32

			CallbackRead();
		}

		if (ev & EV_IOWRITE)
		{
#ifdef _WIN32
#else
			HandleWrite();
#endif // _WIN32
			CallbackWrite();
		}
	}


	int Connecter::WriteBuffer(const char* buf, int len)
	{
		if (!m_pWBuffer)
			return -1;

		m_mutex.Lock();

		uint32 written = m_pWBuffer->WriteBuffer(buf, len);

		m_mutex.UnLock();

		if (0 >= written)
			return -1;

		EventCentre* pCentre = GetCentre();

		if (!pCentre)
			return -1;

		SetCurEv(GetCurEv() | EV_IOWRITE);

		pCentre->PushActiveEv(this);

		return written;
	}


	int Connecter::Write(const char* buf, int len)
	{
		Socket& s = GetSocket();

		int written = 0;
		while (written < len)
		{
			int writelen = s.Send(buf + written, len - written);
			if (0 >= writelen)
				break;

			written += writelen;
		}

		return written;
	}


	int Connecter::HandleRead()
	{
		if (!m_pRBuffer)
			return -1;

		m_mutex.Lock();

		Socket& s = GetSocket();

		socket_unread_t unread = s.GetUnreadByte();
		if (0 >= unread)
			return unread;

		while (0 < unread)
		{
			uint32 size = 0;

			char* buf = m_pRBuffer->GetWriteBuffer(&size);
			if (!buf)
				break;

			int read = s.Recv(buf, size);

			if (0 >= read)
			{
				CancelEvent();
				break;
			}

			m_pRBuffer->MoveWriteBuffer(read);

			unread -= read;
		}

		m_mutex.UnLock();

		return 0;
	}


	int Connecter::HandleWrite()
	{
		if (!m_pWBuffer)
			return -1;

		m_mutex.Lock();

		Socket& s = GetSocket();

		uint32 size = m_pWBuffer->GetReadSize();
		uint32 readSize = 0;

		while (0 > size)
		{
			char* buf = m_pWBuffer->GetWriteBuffer(&readSize);
			if (!buf)
				break;

			int sendSize = 0;
			do
			{
				sendSize = s.Send(buf, readSize);
				readSize -= sendSize;

			} while (readSize > 0);
		}

		m_mutex.UnLock();

		return 0;
	}


//#ifdef _WIN32
//	void AsynConnecter::Handle()
//	{
//		uint32 ev = GetCurEv();
//
//		if (ev & EV_IOREAD)
//		{
//			AsynRead();
//
//			CallbackRead();
//		}
//
//		if (ev & EV_IOWRITE)
//		{
//			AsynWrite();
//
//			CallbackWrite();
//		}
//	}


	int Connecter::AsynRead()
	{
		if (!m_pROverlapped)
		{
			CancelEvent();
			return -1;
		}

		Socket& s = GetSocket();

		m_mutex.Lock();

		if (INVALID_IOCP_RET != m_pROverlapped->asynRet && 0 == m_pROverlapped->bytes/*m_pROverlapped->databuf.len*/)
		{
			printf("AsynRead close socket[%d]\n", s.GetFd());
			CancelEvent();
			return -1;
		}
		
		//收完数据后调整buffer的位置
		if(0 < m_pROverlapped->bytes/*m_pROverlapped->databuf.len*/)
			m_pRBuffer->MoveWriteBuffer(m_pROverlapped->databuf.len);

		uint32 size = 0;
		m_pROverlapped->databuf.buf = m_pRBuffer->GetWriteBuffer(&size);
		m_pROverlapped->databuf.len = size;

		m_pROverlapped->fd = s.GetFd();

		DWORD bytesRead = 0;
		DWORD flags = 0;

		int ret = WSARecv(m_pROverlapped->fd, &m_pROverlapped->databuf, 1, &bytesRead, &flags, &m_pROverlapped->overlapped, NULL);
		if (ret)
		{
			if(GetLastError() != WSA_IO_PENDING)
				CancelEvent();
		}

		m_mutex.UnLock();

		return 0;
	}


	int Connecter::AsynWrite()
	{

		return 0;
	}

//#endif
}


namespace chaos
{
	void TimerEvent::Handle()
	{
		printf("test!\n");
	}
}
