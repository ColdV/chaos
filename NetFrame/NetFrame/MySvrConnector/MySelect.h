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

class MySelect : public MySocketIO
{
public:
	MySelect(int max_socket);
	virtual ~MySelect();

	virtual int initIO(const char* ip, int port);
	//virtual void run();
	virtual void WaitEvent();

protected:
	virtual int addSocket(const uint32 fd, const MySocket& ms, fd_set* rfds, fd_set* sfds, fd_set* efds);
	void delScoket(const uint32 fd);
	void CollectEvent(const fd_set& rfds, const fd_set& sfds, const fd_set& efds);

private:
	fd_set m_rfds;
	fd_set m_sfds;
	fd_set m_efds;
};