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
	virtual int InitIO(const char* ip, int port) = 0;

	virtual void WaitEvent() = 0;

	virtual void HandleEvent(const IOEvent& ioEvent) = 0;

public:
	const std::map<uint32, MySocket>& GetFds() const { return m_sockets; }

	uint32 GetMaxFd() const { return m_max_socket; }

	uint32 GetEventSize() const { return m_event.size(); }

	bool EventEmpty() const { return m_event.empty(); }

	const IOEvent& GetIOEvent() const { return m_event.front(); }

	void DelIOEvent() { m_event.pop(); }


protected:
	virtual void addIOEvent(const IOEvent& ioEvent) { m_event.push(ioEvent); }

protected:
	std::map<uint32, MySocket>	m_sockets;
	uint32 m_max_socket;
	char m_recv_buf[MAX_RECV_BUF_SIZE];
	std::queue<IOEvent> m_event;
};


MySocketIO* CreateSocketIO(int max_fd, IOType ioType  = SI_SELECT);