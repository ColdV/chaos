/************C++ Header File****************
#
#	Filename: MySelect.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:22
#	Last Modified: 2018-08-11 17:40:22
*******************************************/

#pragma once

#include "MySocketIO.h"
#include "../common/single_templete.h"

class MySelect : public MySocketIO
{
public:
	static MySelect& Instance(int max_socket);

	virtual ~MySelect();

	virtual int InitIO(const char* ip, int port);
	//virtual void run();
	virtual void WaitEvent();
	virtual void HandleEvent(const IOEvent& ioEvent);

protected:
	MySelect(int max_socket);

	virtual int addSocket(const uint32 fd, const MySocket& ms, fd_set* rfds, fd_set* wfds, fd_set* efds);
	void delScoket(const uint32 fd);
	void CollectEvent(const fd_set& rfds, const fd_set& wfds, const fd_set& efds);

private:
	fd_set m_rfds;
	fd_set m_wfds;
	fd_set m_efds;
};
