#include <stdio.h>
#include "log/Logger.h"
#include "stdafx.h"
#include "net/Event.h"
#include <functional>
#include "common.h"
#include "net/Timer.h"

#ifdef _WIN32

#include <Psapi.h>
#include<crtdbg.h>

#pragma comment(lib,"psapi.lib")

#define _CRTDBG_MAP_ALLOC

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG


void ShowMemUse()
{
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
	LOG_DEBUG("ÄÚ´æÊ¹ÓÃ:%d", pmc.WorkingSetSize);
}

#endif // _WIN32


struct Client
{
	socket_t fd;
	uint64 totalReadSize;
	uint64 totalWriteSize;
};

class Server
{
public:
	Server() : m_totalSendSize(0), m_totalRecvSize(0) {}
	~Server() {}

	void ListenCb(chaos::Listener* ev, chaos::Connecter*, void* userdata);

	void ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void TimerCb(chaos::Event* pev, short ev, void* userdata);

	void EventCb(chaos::Event* pev, short ev, int errcode, void* userdata);

private:
	uint64 m_totalSendSize;
	uint64 m_totalRecvSize;
	std::unordered_map<socket_t, Client> m_clients;
};


void Server::ListenCb(chaos::Listener* pListener, chaos::Connecter* pConner, void* userdata)
{
	if (!pConner)
	{
		printf("erro new conn!\n");
		return;
	}

	socket_t connfd = pConner->GetSocket().GetFd();

	pConner->SetCallback(std::bind(&Server::ReadCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(&Server::WriteCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL, NULL);

	pConner->SetErrCallback(std::bind(&Server::EventCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), NULL);

	if (m_clients.find(connfd) == m_clients.end())
		m_clients.insert(std::make_pair(connfd, Client{ connfd, 0 }));

	//LOG_DEBUG("socket:%d, newsocket:%d", pListener->GetSocket().GetFd(), pConner->GetSocket().GetFd());
}


void Server::ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	//LOG_DEBUG("read socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);

	if (0 == nTransBytes)
	{
		ev->CancelEvent();
		return;
	}

	int readable = ev->GetReadableSize();
	char* buf = new char[readable];
	if (!buf)
		return;

	memset(buf, 0, readable);

	ev->ReadBuffer(buf, readable);

	if (0 >= ev->WriteBuffer(buf, readable))
	{
		LOG_DEBUG("wirte buffer faled!");
		return;
	}

	socket_t fd = ev->GetSocket().GetFd();
	Client& c = m_clients.find(fd)->second;
	c.totalReadSize += nTransBytes;

	delete[] buf;
}


void Server::WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	socket_t fd = ev->GetSocket().GetFd();
	Client& c = m_clients.find(fd)->second;
	c.totalWriteSize += nTransBytes;

	if (nTransBytes == 0)
	{
		LOG_DEBUG("write socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);
	}
}


void Server::TimerCb(chaos::Event* pev, short ev, void* userdata)
{
	LOG_DEBUG("timer:%d call back", pev->GetEvKey().timerId);
}


void Server::EventCb(chaos::Event* pev, short ev, int errcode, void* userdata)
{
	if (ev & (EV_CANCEL | EV_ERROR))
	{
		if (pev->GetEv() & chaos::IO_CARE_EVENT)
		{
			m_clients.erase(pev->GetEvKey().fd);
			printf("cancel connecter:%d\n", pev->GetEvKey().fd);
		}

		pev->CancelEvent();
	}
}


#define IP "127.0.0.1"
#define PORT 3307


int main()
{
	//---------net test begin-----------//

#ifdef _WIN32
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
	//_CrtSetBreakAlloc(151);		//270 152 151
#endif // _WIN32

#ifndef _WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return 1;
#endif // !_WIN32


	Logger& log = Logger::Instance();
	log.Init("../log", "log", 0);

	Server t;

	chaos::EventCentre* p = new chaos::EventCentre;

	//if (0 != p->Init())
	//{
	//	printf("init event centre failed!\n");
	//	return 0;
	//}

	auto cb = std::bind(&Server::ListenCb, &t, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	chaos::Listener* ev = chaos::Listener::CreateListener(NULL, PORT, 0, TCP_OPT_TCP_NODELAY, cb);
	if (!ev)
	{
		printf("create listener failed!\n");
		return 0;
	}

	p->RegisterEvent(ev);

	//chaos::TimerEvent* timerEv = new chaos::TimerEvent(3, true);
	//timerEv->SetEventCallback(std::bind(&Server::TimerCb, &t, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);
	//p->RegisterEvent(timerEv);

	p->EventLoop();

	delete p;
	//delete timerEv;

	//---------net test end-----------//

	return 0;
}