#pragma once

#include "MySocket.h"
#include "NetDrive.h"
#include "MinHeap.h"
#include "Timer.h"
#include "map"


struct SocketCmp
{
	bool operator()(const MySocket& l, const MySocket& r) const
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
	//typedef void (*EventCb)(MySocket ev, void* userData);

	//struct EventHandle
	//{
	//	MySocket*	evSrc;
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

	//void ProcessFdRead(MySocket* pSocket);

	//void ProcessFdWrite(MySocket* pSocket);


	//ev为该mgr感兴趣的事件通过“|”的组合
	void AddEventMgr(int ev, EventMgr& evMgr);


private:
	NetDrive*	m_base;
	NetDrive* m_scheduler;
	Timer		m_timer;
	std::multimap<int, Event> m_ioEvents;
	std::multimap<MySocket, Event, SocketCmp> m_;
	std::multimap<int, EventMgr> m_evMgr;		
};