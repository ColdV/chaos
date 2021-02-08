#include <stdio.h>
#include "log/Logger.h"
#include "stdafx.h"
#include "net/Event.h"
#include <functional>
#include "common.h"
#include "net/Timer.h"
#include "thread/ThreadPool.h"
#include "db/dbredis/DBRedis.h"
#include "db/DBPool.h"

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



#define IP "127.0.0.1"
#define PORT 3307




struct Client
{
	socket_t fd;
	uint64 totalReadSize;
	uint64 totalWriteSize;
};

class Server
{
public:
	Server();
	~Server() {}

	void Start();

	void ListenCb(chaos::Listener& ev, socket_t fd);

	void ReadCb(chaos::Connecter& ev, int nTransBytes);

	void WriteCb(chaos::Connecter& ev, int nTransBytes);

	void Tick(chaos::TimerEvent& timerEvent);

	void EventCb(chaos::Event& pev, short ev, int errcode);

private:
	uint64 m_totalSendSize;
	uint64 m_totalRecvSize;
	std::unordered_map<socket_t, Client> m_clients;

	std::unique_ptr<chaos::EventCentre> m_centre;
	std::unique_ptr<chaos::EventCentrePool> m_centrePool;
};


Server::Server() :
	m_totalSendSize(0),
	m_totalRecvSize(0),
	m_centre(new chaos::EventCentre),
	m_centrePool(new chaos::EventCentrePool(4))
{
}


void Server::Start()
{
	auto cb = std::bind(&Server::ListenCb, this, std::placeholders::_1, std::placeholders::_2);

	std::shared_ptr<chaos::Listener> ev = chaos::Listener::CreateListener(NULL, PORT, 0, TCP_OPT_TCP_NODELAY, cb);
	if (!ev)
	{
		printf("create listener failed!\n");
		return;
	}

	//===================use event pool===================//
	m_centrePool->Start([ev, this](chaos::EventCentrePool* pPool)
		{
			pPool->RegisterEvent(ev);

			//std::shared_ptr<chaos::TimerEvent> timerEv(new chaos::TimerEvent(1000, std::bind(&Server::Tick, this, std::placeholders::_1),
			//	true));
			//pPool->RegisterEvent(timerEv);
		}
	);
	//===================use event pool===================//

	//m_centre->RegisterEvent(ev);

	std::shared_ptr<chaos::TimerEvent> timerEv(new chaos::TimerEvent(3000, std::bind(&Server::Tick, this, std::placeholders::_1),
		true));
	m_centre->RegisterEvent(timerEv);

	m_centre->EventLoop();
}


void Server::ListenCb(chaos::Listener& listener, socket_t fd)
{
	std::shared_ptr<chaos::Connecter> newconn(new chaos::Connecter(fd));
	newconn->SetReadCallback(std::bind(&Server::ReadCb, this, std::placeholders::_1, std::placeholders::_2));
	newconn->SetWriteCallback(std::bind(&Server::WriteCb, this, std::placeholders::_1, std::placeholders::_2));
	newconn->SetErrCallback(std::bind(&Server::EventCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	m_centre->RegisterEvent(newconn);

	if (m_clients.find(fd) == m_clients.end())
		m_clients.insert(std::make_pair(fd, Client{ fd, 0 }));
}


void Server::ReadCb(chaos::Connecter& ev, int nTransBytes)
{
	//LOG_DEBUG("read socket:%d trans bytes:%d", ev.GetSocket().GetFd(), nTransBytes);

	if (0 == nTransBytes)
	{
		ev.CancelEvent();
		return;
	}

	int readable = ev.GetReadableSize();
	char* buf = new char[readable];

	memset(buf, 0, readable);

	ev.ReadBuffer(buf, readable);

	if (0 >= ev.WriteBuffer(buf, readable))
	{
		LOG_DEBUG("wirte buffer faled!");
		return;
	}

	socket_t fd = ev.GetSocket().GetFd();
	Client& c = m_clients.find(fd)->second;
	c.totalReadSize += nTransBytes;

	delete[] buf;
}


void Server::WriteCb(chaos::Connecter& ev, int nTransBytes)
{
	socket_t fd = ev.GetSocket().GetFd();
	Client& c = m_clients.find(fd)->second;
	c.totalWriteSize += nTransBytes;

	if (nTransBytes == 0)
	{
		LOG_DEBUG("write socket:%d trans bytes:%d", ev.GetSocket().GetFd(), nTransBytes);
	}
}


void Server::Tick(chaos::TimerEvent& timerEvent)
{
	//LOG_DEBUG("timer:%d call back!", timerEvent.GetEvKey().timerId);
	printf("timer:%d call back!, time:%d\n", timerEvent.GetEvKey().timerId, time(NULL));
}


void Server::EventCb(chaos::Event& evt, short ev, int errcode)
{
	if (ev & (EV_CANCEL | EV_ERROR))
	{
		if (evt.GetEv() & chaos::IO_CARE_EVENT)
		{
			m_clients.erase(evt.GetEvKey().fd);
			printf("cancel connecter:%d\n", evt.GetEvKey().fd);
		}

		evt.CancelEvent();
	}
}



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

	//---------net test end-----------//


	//---------db test start---------//
	//chaos::db::DBRedis redis("47.116.73.175", 6379);

	//if (!redis.Connect())
	//{
	//	printf("connect redis failed!\n");
	//	return 0;
	//}

	//chaos::db::DBRedisResult result;
	//redis.Query("get test", &result);

	chaos::db::DBConfig config;
	memset(&config, 0, sizeof(config));

	strcpy(config.redisConfig.dbip, "47.116.73.175");
	config.redisConfig.dbport = 6379;

	chaos::db::DBPool dbPool(config, chaos::db::DBT_REDIS);
	dbPool.Start();
	dbPool.Query("GET test",
		[](const std::string& cmd, chaos::db::DBResultBase& result, int errorno)
		{
			chaos::db::DBRedisResult& redisResult = (chaos::db::DBRedisResult&)result;
			const std::string s = redisResult.String();
			printf("%s\n", s.c_str());
		}
	);

	//---------db test end---------//

	t.Start();

	return 0;
}