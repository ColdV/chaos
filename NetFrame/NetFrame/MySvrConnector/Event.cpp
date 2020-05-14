#include "Event.h"
#include "Timer.h"
#include <stdio.h>


namespace NetFrame
{
	EventCentre::EventCentre() :
		m_pNetDrive(0),
		m_pTimer(0)
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
		m_pNetDrive = NetDrive::AdapterNetDrive();
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
		while (true)
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

	int EventCentre::RegisterEvent(Event* ev)
	{
		if (!ev)
			return -1;

		uint32 iEv = ev->GetEv();

		const EventKey* pEvKey = ev->GetEvKey();

		if (pEvKey)
		{
			if (iEv & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT)
			   && !(iEv & ~(EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT))
				&& m_pNetDrive)
			{
				m_netEvs.insert(std::make_pair(pEvKey->fd, ev));
				m_pNetDrive->AddFd(pEvKey->fd, iEv);
			}
			else if (iEv & EV_TIMEOUT && !(iEv & ~EV_TIMEOUT) && m_pTimer)
			{
				m_timerEvs.insert(std::make_pair(pEvKey->timerId, ev));
				m_pTimer->AddTimer((TimerEvent*)ev);
			}
			else if (iEv & EV_SIGNAL && !(iEv & ~EV_SIGNAL))
				m_signalEvs.insert(std::make_pair(pEvKey->signal, ev));
			else
				return -1;
		}

		return 0;
	}

	int EventCentre::CancelEvent(Event* ev)
	{
		if (!ev)
			return -1;

		uint32 iEv = ev->GetEv();

		const EventKey* pEvKey = ev->GetEvKey();

		if (pEvKey)
		{
			if (iEv & (EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT) && m_pNetDrive)
			{
				m_netEvs.erase(pEvKey->fd);
				m_pNetDrive->DelFd(pEvKey->fd);
			}
			else if (iEv & EV_TIMEOUT && m_pTimer)
			{
				m_timerEvs.erase(pEvKey->timerId);
				m_pTimer->DelTimer((TimerEvent*)ev);
			}
			else if (iEv & EV_SIGNAL)
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

		const std::list<FdEvent>& evs = m_pNetDrive->GetActives();

		for (auto it = evs.begin(); it != evs.end(); ++it)
		{
			auto evIt = m_netEvs.find(it->fd);
			if (evIt == m_netEvs.end())
				continue;

			m_activeEvs.push_back(evIt->second);
		}

		m_pNetDrive->ResetActives();

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

	void NetEvent::Handle()
	{
		uint32 ev = GetCurEv();

		if (!m_pSocket)
			return;

		if (ev & EV_IOREAD)
		{
			uint32 fdType = m_pSocket->GetFdType();

			if (fdType == SKT_LISTEN)
			{
				HandleListen();
			}
			else if (fdType == SKT_CONNING)
			{
				HandleRead();
			}
		}

		if (ev & EV_IOWRITE)
		{
			HandleWrite();
		}
	}


	int NetEvent::HandleListen()
	{
		if (!m_pSocket)
			return -1;

		Socket* pNewSock = m_pSocket->Accept2();
		if (!pNewSock)
			return -1;

		EventKey* pKey = new EventKey();
		if (!pKey)
			return -1;

		pKey->fd = pNewSock->GetFd();

		EventCentre* pCentre = GetCentre();
		if (!pCentre)
			return -1;

		NetEvent* pNewEv = new NetEvent(pCentre, pNewSock, EV_IOREAD | EV_IOWRITE, pKey);
		if (!pNewEv)
			return -1;

		pCentre->RegisterEvent(pNewEv);

		return 0;
	}


	int NetEvent::HandleRead()
	{
		if (!m_pSocket || !m_pRBuffer)
			return -1;

		int nRet = m_pRBuffer->ReadFd(m_pSocket);
		if (0 >= nRet)
		{
			m_pSocket->Close();

			EventCentre* pCentre = GetCentre();
			if (pCentre)
				pCentre->CancelEvent(this);
		}

		return nRet;
	}


	int NetEvent::HandleWrite()
	{
		return m_pSocket && m_pWBuffer ?
			m_pWBuffer->WriteFd(m_pSocket) :
			-1;
	}
}


namespace NetFrame
{
	void TimerEvent::Handle()
	{
		printf("test!\n");
	}
}
