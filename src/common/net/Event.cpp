#include "Event.h"
#include "Timer.h"
#include <stdio.h>
#include <stdexcept>
#include "../log/Logger.h"

namespace chaos
{
	Event::Event(/*EventCentre* pCentre,*/ short ev, const EventKey& evKey) :
		//m_pCenter(pCentre),
		m_pCenter(NULL),
		m_ev(ev)
		//m_curEv(0)
	{
		memcpy(&m_evKey, &evKey, sizeof(EventKey));
	}


	Event::Event() :
		m_pCenter(NULL),
		m_ev(0)
		//m_curEv(0)
	{
		memset(&m_evKey, 0, sizeof(m_evKey));
	}


	void Event::CancelEvent()
	{
		if (m_pCenter)
		{
			m_pCenter->CancelEvent(this);
			SetCenter(NULL);
		}
	}


	void Event::CallErr(int errcode)
	{
		if (m_callback)
			m_errCallback(this, errcode, m_errUserdata);
		else
			CancelEvent();
	}

}	//namespace chaos



namespace chaos
{
	EventCentre::EventCentre() :
		m_pPoller(0),
		m_pTimer(0),
		m_running(false),
		m_evcount(0),
		m_isInit(false)
	{
	}


	EventCentre::~EventCentre()
	{
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

			if (0 != m_pPoller->Launch(loopTickTimeMs))
			{
				printf("called poller failed!\n");
				break;
			}

			if (0 != SignalDispatch())
				break;

			m_pTimer->DispatchTimer();

			if (0 != ProcessActiveEvent())
			{
				printf("process active event failed!\n");
				break;
			}
		}

		m_running = false;

		return 0;
	}


	int EventCentre::ProcessActiveEvent()
	{
		MutexGuard lock(m_mutex);

		while (!m_activeEvs.empty())
		{
			Event* pev = m_activeEvs.front();
			m_activeEvs.pop();
			if (!pev)
				continue;

			if (pev->GetCurEv() & EV_CANCEL)
			{
				delete pev;
				continue;
			}

			pev->Handle();

			pev->Callback();

			//清除此次已处理的事件
			//pev->SetCurEv(0);
			pev->PopCurEv();
		}

		return 0;
	}


	int EventCentre::RegisterEvent(Event* pEvent)
	{
		//if (!pEvent)
		//	return -1;

		if (!pEvent || (pEvent->GetCentre() && this != pEvent->GetCentre()))
		{
			return -1;
		}

		uint32 ev = pEvent->GetEv();

		const EventKey& evKey = pEvent->GetEvKey();

		int ret = 0;

		MutexGuard lock(m_mutex);

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
		{
			return -1;
		}

		--m_evcount;

		pEvent->SetCenter(NULL);

		PushActiveEv(pEvent, EV_CANCEL);

		return 0;
	}


	void EventCentre::PushActiveEv(Event* pEvent, short ev)
	{
		if (!pEvent)
			return;

		if (!(pEvent->GetEv() & ev) && ev != EV_CANCEL)
			return;

		//pEvent->SetCurEv(pEvent->GetCurEv() | ev);
		pEvent->PushCurEv(ev);

		PushActiveEv(pEvent);
	}


	int EventCentre::SignalDispatch()
	{
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

#ifdef IOCP_ENABLE		
		//if (!m_acceptOls)
		//	return -1;

		//for (int i = 0; i < INIT_ASYNACCEPTING; ++i)
		//{
		//	AsynAccept(&m_acceptOls[i]);
		//}
#endif // IOCP_ENABLE

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
			while (1)
			{
				socket_t connfd = s.Accept();
				if (0 > connfd || errno == EAGAIN || connfd == INVALID_SOCKET)
					break;

				//Connecter* pConner = new Connecter(pCentre, connfd);
				//if (!pConner)
				//{
				//	printf("accept assign new connecter failed!\n");
				//	return;
				//}

				//int ret = pCentre->RegisterEvent(pConner);
				//if (0 != ret)
				//	return;

				CallListenerCb(/*pConner*/connfd);
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
				return ret;
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
			printf("getsocketname failed:%d\n", WSAGetLastError());
			return WSAGetLastError();
		}

		int type = 0;
		int typelen = sizeof(type);
		if (getsockopt(listenfd, SOL_SOCKET, SO_TYPE, (char*)&type, &typelen))
		{
			printf("getsockopt failed:%d\n", WSAGetLastError());
			return WSAGetLastError();
		}

		socket_t acceptfd = socket(addr.sin_family, type, 0);
		
		if (!IOCP::AcceptEx)
		{
			printf("AcceptEx is null!\n");
			return -1;
		}

		lo->acceptfd = acceptfd;
		lo->overlapped.fd = listenfd;
		DWORD pending = 0;

		if (IOCP::AcceptEx(listenfd, lo->acceptfd, lo->overlapped.databufs[0].buf, 0, lo->overlapped.databufs[0].len,
			lo->overlapped.databufs[0].len, &pending, &lo->overlapped.overlapped))
		{
			IocpListenCallback(&lo->overlapped.overlapped, 0, 1, true);
		}
		else
		{
			int err = WSAGetLastError();
			if (ERROR_IO_PENDING != err)
				return err;
		}

		return 0;
	}


	void Listener::IocpListenCallback(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok)
	{
		if (!ok)
		{
			//printf("GetQueuedCompletionStatus failed:%d\n", WSAGetLastError());
			return;
		}

		if (!o)
		{
			//printf("GetQueuedCompletionStatus result overlapped is null!\n");
			return;
		}
		
		EventCentre* pCentre = GetCentre();
		if (!pCentre)
			return;

		Socket& s = GetSocket();

		LPACCEPT_OVERLAPPED lo = (LPACCEPT_OVERLAPPED)o;
		socket_t acceptedfd = lo->acceptfd;		//已经连接成功的fd
		socket_t listenfd = GetSocket().GetFd();

		if (0 != setsockopt(lo->acceptfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
			(char*)&listenfd, sizeof(listenfd)))
		{
			printf("setsockopt failed:%d\n", WSAGetLastError());
			return;
		}

		//准备投递下一次AcceptEx
		m_acceptedq.push(lo);
		pCentre->PushActiveEv(this, EV_IOREAD);

		CallListenerCb(/*pConner*/acceptedfd);

	}
#endif // IOCP_ENABLE

}


namespace chaos
{

	Connecter::Connecter(socket_t fd) :
		NetEvent(EV_IOREAD | EV_IOWRITE, fd),
		m_readcb(NULL),
		m_readCbArg(NULL),
		m_writecb(NULL),
		m_writeCbArg(NULL)
	{
		m_pRBuffer = new Buffer;
		m_pWBuffer = new Buffer;

#ifdef IOCP_ENABLE
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

#endif // IOCP_ENABLE

		SetRegisterCallback(std::bind(&Connecter::RegisterCallback, this, std::placeholders::_1));

	}


	Connecter::~Connecter()
	{
		LOG_DEBUG("destroy connecter:%p\n", this);
		if (m_pRBuffer)
			delete m_pRBuffer;

		if (m_pWBuffer)
			delete m_pWBuffer;

#ifdef IOCP_ENABLE
		if (m_pROverlapped)
			delete m_pROverlapped;

		if (m_pWOverlapped)
			delete m_pWOverlapped;
#endif // IOCP_ENABLE
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


	int Connecter::WriteBuffer(const char* buf, int size)
	{
		if (!m_pWBuffer)
			return -1;

		uint32 written = m_pWBuffer->WriteBuffer(buf, size);

		if (0 >= written)
			return -1;

		EventCentre* pCentre = GetCentre();

		if (!pCentre)
			return -1;

		pCentre->PushActiveEv(this, EV_IOWRITE);

		return written;
	}


	int Connecter::Write(const char* buf, int size)
	{
		Socket& s = GetSocket();

		int written = 0;
		while (written < size)
		{
			int writelen = s.Send(buf + written, size - written);
			if (0 >= writelen)
				break;

			written += writelen;
		}

		return written;
	}


	int Connecter::ReadBuffer(char* buf, int size)
	{
		if (!m_pRBuffer)
			return 0;

		int readed = m_pRBuffer->ReadBuffer(buf, size);

		return readed;
	}


	void Connecter::SetCallback(const NetCallback& readcb, void* readCbArg, const NetCallback& writecb, void* writeCbArg)
	{
		if (readcb)
		{
			m_readcb = readcb;
			m_readCbArg = readCbArg;
#ifdef IOCP_ENABLE
			//AsynRead();
#endif // IOCP_ENABLE
		}

		if (writecb)
		{
			m_writecb = writecb;
			m_writeCbArg = writeCbArg;
#ifdef IOCP_ENABLE
			//AsynWrite();
#endif // IOCP_ENABLE
		}
	}


	int Connecter::HandleRead()
	{
		if (!m_pRBuffer)
			return -1;

		Socket& s = GetSocket();

		socket_unread_t unread = s.GetUnreadByte();
		int transferBytes = 0;

		if (0 >= unread)
			return unread;

		while (0 < unread)
		{
			uint32 size = 0;

			char* buf = m_pRBuffer->GetWriteBuffer(&size);
			if (!buf)
				break;

			int read = s.Recv(buf, size);

			if (0 >= read && (errno != EINTR || errno != EWOULDBLOCK || errno != EAGAIN))
			{
				//CancelEvent();
				//CallErr(read);
				//GetSocket().Close();
				transferBytes = 0;
				break;
			}

			m_pRBuffer->MoveWriteBufferPos(read);

			unread -= read;
			transferBytes += read;
		}

		CallbackRead(transferBytes);

		return 0;
	}


	int Connecter::HandleWrite()
	{
		if (!m_pWBuffer)
			return -1;

		Socket& s = GetSocket();

		uint32 size = m_pWBuffer->GetReadSize();
		int tranferBytes = 0;

		while (0 > size)
		{
			uint32 readSize = 0;
			char* buf = m_pWBuffer->GetWriteBuffer(&readSize);
			if (!buf)
				break;

			int sendSize = 0;
			do
			{
				sendSize = s.Send(buf, readSize);
				size -= sendSize;
				tranferBytes += sendSize;

				if (0 >= sendSize && (errno != EINTR || errno != EWOULDBLOCK || errno != EAGAIN))
					break;
				
			} while (readSize > 0);
		}

		CallbackWrite(tranferBytes);

		return 0;
	}


	void Connecter::RegisterCallback(int ret)
	{
		if (0 != ret)
			return;

#ifdef IOCP_ENABLE
		AsynRead();
		AsynWrite();
#endif // IOCP_ENABLE

	}


#ifdef IOCP_ENABLE

	int Connecter::AsynRead()
	{
		if (!m_pROverlapped || !m_pRBuffer)
		{
			//CancelEvent();
			CallErr(-1);
			return -1;
		}

		Socket& s = GetSocket();

		uint32 size = 0;
		m_pROverlapped->databufs[0].buf = m_pRBuffer->GetWriteBuffer(&size);
		m_pROverlapped->databufs[0].len = size;

		m_pROverlapped->fd = s.GetFd();

		DWORD bytesRead = 0;
		DWORD flags = 0;

		int ret = WSARecv(m_pROverlapped->fd, &m_pROverlapped->databufs[0], 1, &bytesRead, &flags, &m_pROverlapped->overlapped, NULL);
		if (ret)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSARecv failed:%d\n", WSAGetLastError());
				//CancelEvent();
				CallErr(ret);
				IocpReadCallback(&m_pROverlapped->overlapped, ret, 0, true);
				return ret;
			}
		}
		//立即返回也会触发 GetQueuedCompletionStatus 所以这里不需要手动调用callback;
		//else
		//{
		//	IocpReadCallback(&m_pROverlapped->overlapped, bytesRead, 0, true);
		//}

		return 0;
	}


	int Connecter::AsynWrite()
	{
		if (!m_pWOverlapped || !m_pWBuffer)
		{
			return -1;
			//CancelEvent();
			CallErr(-1);
		}

	
		uint32 readySize = m_pWBuffer->GetReadSize();

		while (0 < readySize)
		{
			uint32 readSize = readySize;
			m_pWOverlapped->databufs[0].buf = m_pWBuffer->ReadBuffer(&readSize);
			m_pWOverlapped->databufs[0].len = readSize;

			if (!m_pWOverlapped->databufs[0].buf || 0 == readSize)
			{	
				printf("error read buffer!\n");
				break;
			}

			DWORD sendBytes = 0;
			DWORD flags = 0;

			int ret = WSASend(GetSocket().GetFd(), &m_pWOverlapped->databufs[0], 1, &sendBytes, flags, &m_pWOverlapped->overlapped, NULL);
			if (ret)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					printf("WSARecv failed:%d\n", WSAGetLastError());
					//CancelEvent();
					IocpWriteCallback(&m_pWOverlapped->overlapped, ret, 0, true);
					return  WSAGetLastError();
				}
				else
				{
					printf("WSASend failed:%d\n", WSAGetLastError());
				}
			}
			//else
			//{
			//	IocpWriteCallback(&m_pWOverlapped->overlapped, sendBytes, 0, true);
			//}
		}

		return 0;
	}


	void Connecter::IocpReadCallback(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok)
	{
		if (!ok)
		{
			//printf("GetQueuedCompletionStatus failed:%d\n", WSAGetLastError());
		}

		if (!o)
		{
			//printf("GetQueuedCompletionStatus result overlapped is null!\n");
			return;
		}

		if (!m_pROverlapped)
		{
			printf("read overlapped is null\n");
			//CancelEvent();
			CallErr(-1);
			return;
		}

		//if (0 == bytes)
		//{
		//	//CancelEvent();
		//	GetSocket().Close();
		//	return;
		//}

		//收完数据后调整buffer下次写入的位置
		m_pRBuffer->MoveWriteBufferPos(bytes);

		EventCentre* pCentre = GetCentre();
		if (!pCentre)
			return;

		//准备投递下一次WSARecv
		if(0 < bytes)
			pCentre->PushActiveEv(this, EV_IOREAD);

		CallbackRead(bytes);
	}


	void Connecter::IocpWriteCallback(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok)
	{
		if (!ok)
		{
			//printf("GetQueuedCompletionStatus failed:%d\n", WSAGetLastError());
			return;
		}

		if (!o)
		{
			//printf("GetQueuedCompletionStatus result overlapped is null!\n");
			return;
		}

		CallbackWrite(bytes);
	}

#endif // IOCP_ENABLE

}


namespace chaos
{
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
