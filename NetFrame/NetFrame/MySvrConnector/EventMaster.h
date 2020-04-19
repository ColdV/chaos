#pragma once

#include "Socket.h"
#include "NetDrive.h"
#include "MinHeap.h"
#include "Timer.h"
#include "map"


struct SocketCmp
{
	bool operator()(const Socket& l, const Socket& r) const
	{
		return l.getSocket() < r.getSocket();
	}
};

union EventOld
{
	SocketIOEvent fdEvent;
	TimerEvent	timerEvent;
};

//struct TimerEvent;

class IOEevntMgr;
 
class EventMgr;

class EventMaster
{
public:
	//typedef void (*EventCb)(Socket ev, void* userData);

	//struct EventHandle
	//{
	//	Socket*	evSrc;
	//	EventCb		listenCb;
	//	EventCb		readCb;
	//	EventCb		writeCb;
	//	EventCb		errCb;
	//};

	EventMaster();			
	~EventMaster();

	int Init();

	int Init(const char* ip, int port);

	int Loop();

	int Stop();

	void RegisterEvent(uint32 fd, EventCb readCb, EventCb writeCb, EventCb listenCb, EventCb errCb, void* userArg);

	int AddEvent(Event& ev);

	void ProcessEvenet();

	void EraseEvent(Event& ev);

	//void ProcessFdRead(Socket* pSocket);

	//void ProcessFdWrite(Socket* pSocket);


	//ev为该mgr感兴趣的事件通过“|”的组合
	void AddEventMgr(int ev, EventMgr& evMgr);


private:
	NetDrive*	m_base;
	NetDrive* m_scheduler;
	Timer		m_timer;
	std::multimap<int, Event> m_ioEvents;
	std::multimap<Socket, Event, SocketCmp> m_;
	std::multimap<int, EventMgr> m_evMgr;		
};