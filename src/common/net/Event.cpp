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


	void EventCentre::EventLoop()
	{
		m_running = true;

		while (m_running)
		{
			if (0 != DispatchEvent())
				break;
		}
	}


	int EventCentre::DispatchEvent()
	{
		int ret = 0;

		if (0 != (ret = TimerDispatch()))
			return ret;

		if (0 != (ret = NetEventDispatch()))
			return ret;

		if (0 != (ret = SignalDispatch()))
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

			//清除此次已处理的事件
			pev->SetCurEv(pev->GetCurEv() & (~EV_IOWRITE));

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

		if (ev & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT)
			&& !(ev & ~(EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
			&& m_pPoller)
		{
			//m_netEvs.insert(std::make_pair(pEvKey->fd, pEvent));
			m_pPoller->AddEvent(pEvent);
		}
		else if (ev & EV_TIMEOUT && !(ev & ~EV_TIMEOUT) && m_pTimer)
		{
			//m_timerEvs.insert(std::make_pair(evKey.timerId, pEvent));
			m_pTimer->AddTimer(pEvent);
		}
		else if (ev & EV_SIGNAL && !(ev & ~EV_SIGNAL))
			m_signalEvs.insert(std::make_pair(evKey.signal, pEvent));
		else
			return -1;

		return 0;
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

		return 0;
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

	int Listener::Listen(const char* ip, int port)
	{
		Socket& s = GetSocket();

		int ret = 0;

		//设置socket为非阻塞
		if (0 > (ret = s.SetNonBlock()))
			return ret;

		if (0 != (ret = s.Bind(ip, port)))
			return ret;

		if (0 != (ret = s.Listen()))
			return ret;

		return ret;
	}


	void Listener::Handle()
	{
		Socket& s = GetSocket();
		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{
			while(1)
			{
				socket_t connfd = s.Accept();
				if (0 > connfd || errno == EAGAIN || connfd == INVALID_SOCKET)
					break;

				EventKey key;

				EventCentre* pCentre = GetCentre();
				if (!pCentre)
					return;

				Event* pNewEv = NULL;
#ifdef _WIN32
				pNewEv = new AsynConnecter(pCentre, connfd);
#else
				pNewEv = new Connecter(pCentre, connfd);
#endif
				if (!pNewEv)
				{
					return;
				}

				pCentre->RegisterEvent(pNewEv);
			} 
		}

	}
}


namespace chaos
{

	void Connecter::Handle()
	{
		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{

			HandleRead();
		}

		if (ev & EV_IOWRITE)
		{
			HandleWrite();
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


#ifdef _WIN32
	void AsynConnecter::Handle()
	{
		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{
			AsynRead();
		}

		if (ev & EV_IOWRITE)
		{
			AsynWrite();
		}
	}


	int AsynConnecter::AsynRead()
	{
		if (!m_pOverlapped)
		{
			CancelEvent();
			return -1;
		}

		Socket& s = GetSocket();

		if (INVALID_IOCP_RET != m_pOverlapped->asynRet && 0 == m_pOverlapped->databuf.len)
		{
			printf("AsynRead close socket[%d]\n", s.GetFd());
			CancelEvent();
			return -1;
		}
		
		//收完数据后调整buffer的位置
		if(0 < m_pOverlapped->databuf.len)
			m_pRBuffer->MoveWriteBuffer(m_pOverlapped->databuf.len);

		uint32 size = 0;
		m_pOverlapped->databuf.buf = m_pRBuffer->GetWriteBuffer(&size);
		m_pOverlapped->databuf.len = size;

		m_pOverlapped->key.fd = s.GetFd();

		DWORD bytesRead = 0;
		DWORD flags = 0;

		int ret = WSARecv(m_pOverlapped->key.fd, &m_pOverlapped->databuf, 1, &bytesRead, &flags, &m_pOverlapped->overlapped, NULL);
		if (ret)
		{
			if(GetLastError() != WSA_IO_PENDING)
				CancelEvent();
		}

		return 0;
	}


	int AsynConnecter::AsynWrite()
	{
		return 0;
	}

#endif
}


namespace chaos
{
	void TimerEvent::Handle()
	{
		printf("test!\n");
	}
}
