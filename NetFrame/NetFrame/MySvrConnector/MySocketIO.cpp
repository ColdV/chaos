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

