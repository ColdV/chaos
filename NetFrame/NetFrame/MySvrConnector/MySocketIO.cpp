/************C++ Source File****************
#
#	Filename: MySocketIO.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:02
#	Last Modified: 2018-08-11 17:40:02
*******************************************/


#include "MySocketIO.h"
#include "MySelect.h"
#include "MyEpoll.h"
#include "MyIOCP.h"

MySocketIO::MySocketIO()
{
	m_sockets.clear();
	m_max_socket = 0;
	memset(m_recv_buf, 0, MAX_RECV_BUF_SIZE);
	m_ioType = SI_SELECT;
	//InitIOThread();

/*
#ifdef _WIN32

	static WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		printf("initialize windows socket failed!\n");
		exit(0);
	}

#endif // _WIN32
*/
}

MySocketIO::~MySocketIO()
{
	printf("delete IO!\n");
}

/*
bool MySocketIO::InitIOThread()
{
	_beginthreadex(NULL, 0, ReadThreadProcess, this, 0, (unsigned int*)&m_r_tid);
	return true;
}

*/
/*
void MySocketIO::addIOEvent(const IOEvent& ioEvent)
{
	switch (ioEvent.sock_event)
	{
	default:
		break;

	case SE_READ:
		//LockReadQueue();
		m_r_event.push(ioEvent);
		//UnLockReadQueue();
		break;
	}
}
*/
/*
#ifdef _WIN32

unsigned int __stdcall MySocketIO::ReadThreadProcess(void* param)
{
	if (!param)
		return -1;

	MySocketIO* pIO = static_cast<MySocketIO*>(param);

	while (true)
	{
		if (!pIO->ReadEventEmpty())
		{
			//pIO->LockReadQueue();

			printf("wait event!event size:%d\n", pIO->GetEventSize());
			pIO->HandleEvent(pIO->GetReadEvent());
			pIO->PopReadEvent();

			//pIO->UnLockReadQueue();
		}
	}

	printf("Ñ­»·ÍË³ö!\n");
}

#endif // _WIN32
*/


SocketIOEvent* MySocketIO::GetEvent(uint32 fd)
{
	auto it = m_events.find(fd);
	if (it != m_events.end())
		return &it->second;

	return NULL;
}


void MySocketIO::AddEvent(uint32 fd, EventCb readCb, EventCb writeCb, EventCb listenCb, EventCb errCb, void* userData)
{
	SocketIOEvent ev;
	ev.eventHandler.readCb = readCb;
	ev.eventHandler.writeCb = writeCb;
	ev.eventHandler.errCb = errCb;
	ev.eventHandler.listenCb = listenCb;
	ev.fd = fd;
	ev.pUserData = userData;

	m_events.insert(std::make_pair(fd, ev));
}


void MySocketIO::ProcessEvent()
{
	while (!EventEmpty())
		//HandleEvent(GetIOEvent());
	{
		const IOEvent& ev = GetIOEvent();

		auto it = m_sockets.find(ev.fd);
		if (it == m_sockets.end())
			break;

		MySocket& sk = it->second;

		switch (ev.sock_event)
		{
		default:
			break;

		case SE_READ:
			if (sk.getType() == SKT_LISTEN)
				ProcessListen(sk);
			else
				ProcessRead(sk);

			break;

		case SE_WRITE:
			ProcessWrite(sk);
			break;

		case SE_EXCEPT:
			ProcessErr(sk);
			break;
		}

		DelIOEvent();
	}


}



void MySocketIO::ProcessListen(MySocket& sk)
{
	MySocket newSocket;
	bool loop = true;
	int acceptNum = 0;
	while (loop)
	{
		int ret = sk.Accept(newSocket);
		if (0 < ret)//(0 < sk.Accept(newSocket))//;
		{
			m_sockets.insert(std::make_pair(newSocket.getSocket(), newSocket));
			AddSocket(newSocket.getSocket());
		}

		else if (0 > ret&& EAGAIN == errno)
		{
			printf("accept connect:%d\n", acceptNum);
			break;
		}

		else
		{
			printf("accept failed!\n");
			break;
		}
		
		++acceptNum;

		if (m_ioType != SI_EPOLL)
			loop = false;

	}
}


void MySocketIO::ProcessRead(MySocket& sk)
{
	bool loop = true;
	int n = 0;
	int recvLen = 0;
	int len = 0;
	while (loop)
	{
#ifndef _WIN32
		ioctl(sk.getSocket(), FIONREAD, &n);
#endif
		len = sk.Recv(m_recv_buf, MAX_RECV_BUF_SIZE);
		if (0 >= len)
		{
			DelSocket(sk.getSocket());
			m_sockets.erase(sk.getSocket());
			break;
		}

		recvLen += len;
		if (recvLen >= n)
			break;

		if (m_ioType != SI_EPOLL)
			loop = false;
	}

}


void MySocketIO::ProcessWrite(MySocket& sk)
{

}

void MySocketIO::ProcessErr(MySocket& sk)
{
	printf("socket[%d] err!\n", sk.getSocket());
	DelSocket(sk.getSocket());
	m_sockets.erase(sk.getSocket());
}


MySocketIO* CreateSocketIO(int max_fd, IOType ioType /*= SI_SELECT*/)
{
	if (FD_SETSIZE >= max_fd && SI_SELECT == ioType)
		return &MySelect::Instance(max_fd);
	
	else
	{
#ifdef _WIN32
		return &MyIOCP::Instance();

#else
		return &MyEpoll::Instance();

#endif // _WIN32
	}

	return NULL;
}

