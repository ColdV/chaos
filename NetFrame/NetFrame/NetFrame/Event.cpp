#include "Event.h"
#include "Timer.h"
#include <stdio.h>


namespace NetFrame
{
	void Event::CancelEvent()
	{
		if (m_pCenter)
			m_pCenter->CancelEvent(this);
	}


	EventCentre::EventCentre() :
		m_pNetDrive(0),
		m_pTimer(0),
		m_running(false)
	{
	}


	EventCentre::~EventCentre()
	{
		if (m_pNetDrive)
			delete m_pNetDrive;

		if (!m_pTimer)
			delete m_pTimer;
	}


	int EventCentre::Init()
	{
		m_pNetDrive = NetDrive::AdapterNetDrive(this);
		if (!m_pNetDrive)
			return -1;

		m_pNetDrive->Init();

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
		for (auto it = m_activeEvs.begin(); it != m_activeEvs.end(); ++it)
		{
			if (!*it)
				continue;

			(*it)->Handle();
		}

		m_activeEvs.clear();

		return 0;
	}


	int EventCentre::RegisterEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		uint32 ev = pEvent->GetEv();

		const EventKey* pEvKey = pEvent->GetEvKey();

		if (pEvKey)
		{
			if (ev & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT)
			   && !(ev & ~(EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
				&& m_pNetDrive)
			{
				//m_netEvs.insert(std::make_pair(pEvKey->fd, pEvent));
				m_pNetDrive->AddEvent(pEvent);
			}
			else if (ev & EV_TIMEOUT && !(ev & ~EV_TIMEOUT) && m_pTimer)
			{
				m_timerEvs.insert(std::make_pair(pEvKey->timerId, pEvent));
				m_pTimer->AddTimer((TimerEvent*)pEvent);
			}
			else if (ev & EV_SIGNAL && !(ev & ~EV_SIGNAL))
				m_signalEvs.insert(std::make_pair(pEvKey->signal, pEvent));
			else
				return -1;
		}

		return 0;
	}


	int EventCentre::CancelEvent(Event* pEvent)
	{
		if (!pEvent)
			return -1;

		uint32 ev = pEvent->GetEv();

		const EventKey* pEvKey = pEvent->GetEvKey();

		if (pEvKey)
		{
			if (ev & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT) && m_pNetDrive)
			{
				//m_netEvs.erase(pEvKey->fd);
				m_pNetDrive->DelEvent(pEvent);
			}
			else if (ev & EV_TIMEOUT && m_pTimer)
			{
				m_timerEvs.erase(pEvKey->timerId);
				m_pTimer->DelTimer((TimerEvent*)pEvent);
			}
			else if (ev & EV_SIGNAL)
				m_signalEvs.erase(pEvKey->signal);
			else
				return -1;
		}

		return 0;
	}


	int EventCentre::NetEventDispatch()
	{
		if (!m_pNetDrive)
			return -1;

		int ret = 0;
		if (0 != (ret = m_pNetDrive->Launch()))
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



namespace NetFrame
{
	void Listener::Handle()
	{
		if (!m_pSocket)
			return;

		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{
			do
			{
				//优化：这里考虑内存分配是在Accept调用中还是在此处分配
				Socket* pNewSock = m_pSocket->Accept();
				if (!pNewSock || 0 > pNewSock->GetFd() ||
					errno == EAGAIN ||
					pNewSock->GetFd() == INVALID_SOCKET)
				{
					delete pNewSock;
					return;
				}

				EventKey* pKey = new EventKey();
				if (!pKey)
					return;

				pKey->fd = pNewSock->GetFd();

				EventCentre* pCentre = GetCentre();
				if (!pCentre)
					return;

				Event* pNewEv = NULL;

#if defined WIN32
				pNewEv = new AsynConnecter(pCentre, pNewSock, EV_IOREAD | EV_IOWRITE, pKey);
#else
				pNewEv = new Connecter(pCentre, pNewSock, EV_IOREAD | EV_IOWRITE, pKey);
#endif
				if (!pNewEv)
				{
					delete pNewSock;
					return;
				}

				pCentre->RegisterEvent(pNewEv);

				SetCurEv(ev & (~EV_IOREAD));

			} while (m_pSocket->Block());
		}

	}
}


namespace NetFrame
{

	void Connecter::Handle()
	{
		if (!m_pSocket)
			return;

		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{

			HandleRead();

			SetCurEv(ev & (~EV_IOREAD));
		}

		if (ev & EV_IOWRITE)
		{
			HandleWrite();
			SetCurEv(ev & (~EV_IOWRITE));
		}
	}



	int Connecter::HandleRead()
	{
		if (!m_pSocket || !m_pRBuffer)
			return -1;

		int nRet = m_pRBuffer->ReadSocket(m_pSocket);
		if (0 >= nRet)
		{
			//m_pSocket->Close();

			CancelEvent();
		}

		return nRet;
	}


	int Connecter::HandleWrite()
	{
		return m_pSocket && m_pWBuffer ?
			m_pWBuffer->WriteSocket(m_pSocket) :
			-1;
	}


	void AsynConnecter::Handle()
	{
		if (!m_pSocket)
			return;

		uint32 ev = GetCurEv();

		if (ev & EV_IOREAD)
		{
			AsynRead();
			SetCurEv(ev & (~EV_IOREAD));
		}

		if (ev & EV_IOWRITE)
		{
			AsynWrite();
			SetCurEv(ev & (~EV_IOWRITE));
		}
	}


	int AsynConnecter::AsynRead()
	{
		if (!m_pOverlapped)
		{
			CancelEvent();
			return -1;
		}

		Socket* s = GetSocket();
		if (!s)
		{
			CancelEvent();
			return -1;
		}

		if (INVALID_IOCP_RET != m_pOverlapped->asynRet && 0 == m_pOverlapped->databuf.len)
		{
			printf("AsynRead close socket[%d]\n", s->GetFd());
			CancelEvent();
			return -1;
		}
		
		//收完数据后调整buffer的位置
		if(0 < m_pOverlapped->databuf.len)
			m_pRBuffer->MoveWriteBuffer(m_pOverlapped->databuf.len);

		uint32 size = 0;
		m_pOverlapped->databuf.buf = m_pRBuffer->GetWriteBuffer(&size);
		m_pOverlapped->databuf.len = size;

		m_pOverlapped->key.fd = s->GetFd();

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
}


namespace NetFrame
{
	void TimerEvent::Handle()
	{
		printf("test!\n");
	}
}
