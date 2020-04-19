/************C++ Header File****************
#
#	Filename: MyEpoll.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:31
#	Last Modified: 2018-08-10 10:12:31
*******************************************/

#ifndef _WIN32

#pragma once

#include "NetDrive.h"
#include <sys/epoll.h>


class MyEpoll : public NetDrive
{
public:

	static MyEpoll& Instance();

	virtual ~MyEpoll() {}

	virtual int InitIO(const char* ip, int port, uint32 max_fd);

	virtual void WaitEvent();

	virtual void HandleEvent(const IOEvent& ioEvent);

	int AddSocket(const Socket& s);

protected:
	MyEpoll();

	virtual void delSocket(const uint32 fd);

	virtual int AddSocket(uint32 fd);

	virtual int DelSocket(uint32 fd);

private:
	int m_max_fd;
	int m_epfd;
	epoll_event* m_events;
};

#endif // !_WIN32
