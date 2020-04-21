/************C++ Source File****************
#
#	Filename: Select.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:25
#	Last Modified: 2018-08-11 17:40:25
*******************************************/


#include "MySelect.h"

namespace NetFrame
{

	Select& Select::Instance(int max_socket)
	{
		static Select mySelect_(max_socket);

		return mySelect_;
	}


	Select::Select(int max_socket)
	{
		m_sockets.clear();
		m_max_socket = max_socket;
		FD_ZERO(&m_rfds);
		FD_ZERO(&m_wfds);
		FD_ZERO(&m_efds);
	}

	Select::~Select()
	{
	}

	int Select::InitIO(const char* ip, int port, uint32 max_fd)
	{
		Socket mSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		int res = 0;

		res = mSocket.Bind(ip, port);
		if (0 != res)
			return res;

		res = mSocket.Listen();
		if (0 != res)
			return res;

		printf("Begin listen port:%d\n", port);

		res = addSocket(mSocket.getSocket(), mSocket, &m_rfds, NULL, NULL);
		//res = AddSocket(mSocket.getSocket());

		if (0 != res)
			return res;

		return res;
	}

	void Select::WaitEvent()
	{

		fd_set rfds = m_rfds;
		fd_set wfds = m_wfds;
		fd_set efds = m_efds;

		int cnt = select(m_max_socket + 1, &rfds, NULL, &efds, 0);

		if (0 > cnt)
		{
			printf("call select failed! code:%d\n", cnt);
			return;
		}

		if (0 == cnt)
			return;//continue;

		CollectEvent(rfds, wfds, efds);
	}


	void Select::CollectEvent(const fd_set& rfds, const fd_set& wfds, const fd_set& efds)
	{
		IOEvent newEvent;

#ifdef _WIN32

		for (uint32 i = 0; i < rfds.fd_count; ++i)
		{
			newEvent.fd = rfds.fd_array[i];
			newEvent.sock_event = SE_READ;

			addIOEvent(newEvent);
		}

		for (uint32 i = 0; i < wfds.fd_count; ++i)
		{
			newEvent.fd = wfds.fd_array[i];
			newEvent.sock_event = SE_WRITE;

			addIOEvent(newEvent);
		}

		for (uint32 i = 0; i < efds.fd_count; ++i)
		{
			newEvent.fd = efds.fd_array[i];
			newEvent.sock_event = SE_EXCEPT;

			addIOEvent(newEvent);
		}

#else
		/*
		//for (uint32 i = 0; i < m_sockets.size(); ++i)
		for(auto it = m_sockets.begin(); it != m_sockets.end(); ++it)
		{
			//if (0 == (rfds >> i))
			//	break;

			//if ((rfds >> i) & 1)

			uint32 fd = it->first;

			if(FD_ISSET(fd, &rfds))
			{
				newEvent.fd = i;
				newEvent.sock_event = SE_READ;

				addIOEvent(newEvent);
			}

			if(FD_ISSET(it->first, &wfds))
			{
				newEvent.fd = i
			}
		}

	//	for (uint32 i = 0; i < m_sockets.size(); ++i)
		for(auto it = m_sockets.begin(); it != m_sockets.end(); ++it)
		{
			//if (0 == (wfds >> i))
			//	break;

			//if ((wfds >> i) & 1)
			if(FD_ISSET(it->first, &wfds))
			{
				newEvent.fd = i;
				newEvent.sock_event = SE_WRITE;

				addIOEvent(newEvent);
			}
		}

		//for (uint32 i = 0; i < m_sockets.size(); ++i)
		for (auto it = m_soc)
		{
			if (0 == (efds >> i))
				break;

			if ((efds >> i) & 1)
			{
				newEvent.fd = i;
				newEvent.sock_event = SE_EXCEPT;

				addIOEvent(newEvent);
			}
		}
	*/

		for (auto it = m_sockets.begin(); it != m_sockets.end(); ++it)
		{
			uint32 fd = it->first;

			if (FD_ISSET(fd, &rfds))
				newEvent.sock_event = SE_READ;

			else if (FD_ISSET(fd, &wfds))
				newEvent.sock_event = SE_WRITE;

			else if (FD_ISSET(fd, &efds))
				newEvent.sock_event = SE_EXCEPT;

			else
				continue;

			newEvent.fd = fd;
			addIOEvent(newEvent);
		}

#endif // _WIN32

	}


	int Select::addSocket(const uint32 fd, const Socket& ms, fd_set* rfds, fd_set* wfds, fd_set* efds)
	{
		if (!m_sockets.insert(std::make_pair(fd, ms)).second)
			return -1;

		if (NULL != rfds)
			FD_SET(fd, rfds);

		if (NULL != wfds)
			FD_SET(fd, wfds);

		if (NULL != wfds)
			FD_SET(fd, efds);

		return 0;
	}


	int Select::AddSocket(uint32 fd)
	{
		FD_SET(fd, &m_rfds);
		FD_SET(fd, &m_efds);

		return 0;
	}

	int Select::DelSocket(uint32 fd)
	{
		FD_CLR(fd, &m_rfds);
		FD_CLR(fd, &m_wfds);
		FD_CLR(fd, &m_efds);

		return 0;
	}

	void Select::delSocket(const uint32 fd)
	{
		m_sockets.erase(fd);
		FD_CLR(fd, &m_rfds);
		FD_CLR(fd, &m_wfds);
		FD_CLR(fd, &m_efds);
	}


	void Select::HandleEvent(const IOEvent& ioEvent)
	{

		//printf("发生事件的FD:%d, 事件类型:%d\n", ioEvent.fd, ioEvent.sock_event);
		std::map<uint32, Socket>::iterator it = m_sockets.find(ioEvent.fd);

		if (it == m_sockets.end())
			return;

		switch (ioEvent.sock_event)
		{
		default:
			break;

		case SE_READ:
		{
			if (it->second.getType() == SKT_LISTEN)
			{
				Socket new_socket;
				if (0 < it->second.Accept(new_socket))
					addSocket(new_socket.getSocket(), new_socket, &m_rfds, NULL, NULL);
				else
				{
					printf("accept 失败!\n");
				}
			}

			else
			{
				if (0 >= it->second.Recv(m_recv_buf, MAX_RECV_BUF_SIZE))
				{
					it->second.Close();
					delSocket(it->first);
				}
			}
		}
		break;

		case SE_WRITE:
			break;

		case SE_EXCEPT:
			printf("socket[%d] except\n", it->first);
			delSocket(it->first);
			//it->second.Close();
			break;

		}

		DelIOEvent();
	}


	void Select::HandleEvent()
	{

	}

}