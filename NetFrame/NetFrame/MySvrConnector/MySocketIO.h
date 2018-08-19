/************C++ Header File****************
#
#	Filename: MySocketIO.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:39:58
#	Last Modified: 2018-08-11 17:39:58	
*******************************************/

#pragma once
#include "MySocket.h"

enum IOType
{
	SI_SELECT = 1,
	SI_EPOLL = 2,
	SI_IOCP = 3
};

enum
{
	MAX_RECV_BUF_SIZE = 1024 * 10,
};

enum SockEvent
{
	SE_READ = 1,
	SE_WRITE = 1 << 1,
	SE_EXCEPT = 1 << 2,
};

struct IOEvent
{
	uint32 fd;
	SockEvent sock_event;
};

class MySocketIO
{
public:
	MySocketIO();
	virtual ~MySocketIO();

public:
	virtual void run() = 0;

	virtual void WaitEvent() = 0;

public:
	const std::map<uint32, MySocket>& getFds() const { return m_sockets; }

	uint32 getMaxFd() const { return m_max_socket; }

	uint32 getEventSize() const { return m_event.size(); }

	const IOEvent& getIOEvent() const { return m_event.front(); }

	void delIOEvent() { m_event.pop(); }


protected:
	virtual int initIO(const char* ip, int port) = 0;

	virtual void addIOEvent(const IOEvent& ioEvent) { m_event.push(ioEvent); }

protected:
	std::map<uint32, MySocket>	m_sockets;
	uint32 m_max_socket;
	char m_recv_buf[MAX_RECV_BUF_SIZE];
	std::queue<IOEvent> m_event;
};