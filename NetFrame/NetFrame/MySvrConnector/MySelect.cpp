/************C++ Source File****************
#
#	Filename: MySelect.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:25
#	Last Modified: 2018-08-11 17:40:25
*******************************************/


#include "MySelect.h"

MySelect::MySelect(int max_socket)
{
	m_sockets.clear();
	m_max_socket = max_socket;	 
	FD_ZERO(&m_rfds);
	FD_ZERO(&m_sfds);
	FD_ZERO(&m_efds);
}

MySelect::~MySelect()
{
}

int MySelect::initIO(const char* ip, int port)
{
	MySocket mSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int res = 0;

	res = mSocket.Bind(ip, port);
	if (0 != res)
		return res;

	res = mSocket.Listen();
	if (0 != res)
		return res;

	res = addSocket(mSocket.getSocket(), mSocket, &m_rfds, NULL, NULL);

	if (0 != res)
		return res;

	return res;
}

void MySelect::WaitEvent()
{
	/*
	while (true)
	{
	*/
		fd_set rfds = m_rfds;
		fd_set sfds = m_sfds;
		fd_set efds = m_efds;

		int cnt = select(m_max_socket + 1, &rfds, NULL, &efds, 0);

		if (0 > cnt)
		{
			printf("call select failed! code:%d\n", cnt);
			return;
		}

		if (0 == cnt)
			return;//continue;

		CollectEvent(rfds, sfds, efds);
		/*
		for (std::map<uint32, MySocket>::iterator it = m_sockets.begin; it != m_sockets.end() && cnt > 0; ++it)
		{
			if (FD_ISSET(it->first, &rfds))
			{
				if (SKT_LISTEN == it->second.getType())
					it->second.Accept();
				else
					it->second.Recv(m_recv_buf, MAX_RECV_BUF_SIZE);

				--cnt;
			}

			else if (FD_ISSET(it->first, &efds))
			{
				it->second.Close();
				--cnt;
			}
		}
		*/
	//}
}

void MySelect::CollectEvent(const fd_set& rfds, const fd_set& sfds, const fd_set& efds)
{
	IOEvent newEvent;

#ifdef _WIN32
	for (int i = 0; i < rfds.fd_count; ++i)
	{
		newEvent.fd = rfds.fd_array[i];
		newEvent.sock_event = SE_READ;

		addIOEvent(newEvent);
	}

	for (int i = 0; i < sfds.fd_count; ++i)
	{
		newEvent.fd = sfds.fd_array[i];
		newEvent.sock_event = SE_WRITE;

		addIOEvent(newEvent);
	}

	for (int i = 0; i < efds.fd_count; ++i)
	{
		newEvent.fd = efds.fd_array[i];
		newEvent.sock_event = SE_EXCEPT;

		addIOEvent(newEvent);
	}

#else



#endif // _WIN32

}


int MySelect::addSocket(const uint32 fd, const MySocket& ms, fd_set* rfds, fd_set* sfds, fd_set* efds)
{
	if (!m_sockets.insert(std::make_pair(fd, ms)).second)
		return -1;

	if (NULL != rfds)
		FD_SET(fd, rfds);

	if (NULL != sfds)
		FD_SET(fd, sfds);

	if (NULL != sfds)
		FD_SET(fd, efds);

	return 0;
}

void MySelect::delScoket(const int fd)
{
	m_sockets.erase(fd);
	FD_CLR(fd, &m_rfds);
	FD_CLR(fd, &m_sfds);
	FD_CLR(fd, &m_efds);
}