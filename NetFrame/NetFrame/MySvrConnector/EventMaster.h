#pragma once

#include "MySocket.h"
#include "MySocketIO.h"


union Event
{
	SocketIOEvent fdEvent;
};


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

	//void SetListenCb(EventCb listenCb) { m_listenCb = listenCb; }
	//void SetReadCb(EventCb readCb) { m_readCb = readCb; }
	//void SetWriteCb(EventCb writeCb) { m_writeCb = writeCb; }
	//void SetErrCb(EventCb errCb) { m_errCb = errCb; }

	void RegisterEvent(uint32 fd, EventCb readCb, EventCb writeCb, EventCb listenCb, EventCb errCb, void* userArg);

	void ProcessEvenet();

	//void ProcessFdRead(MySocket* pSocket);

	//void ProcessFdWrite(MySocket* pSocket);

private:

	MySocketIO*	m_base;
	//std::queue<Event> m_active;
	//EventCb		m_listenCb;
	//EventCb		m_readCb;
	//EventCb		m_writeCb;
	//EventCb		m_errCb;
};