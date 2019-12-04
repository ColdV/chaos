/************C++ Source File****************
#
#	Filename: MyEpoll.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:46
#	Last Modified: 2018-08-10 10:12:46
*******************************************/

#ifndef _WIN32

#include "MyEpoll.h"

MyEpoll& MyEpoll::Instance()
{
	static MyEpoll myEpoll_;
	return myEpoll_;
}

MyEpoll::MyEpoll()
{
}


int MyEpoll::InitIO(const char* ip, int port, uint32 max_fd)
{
	MySocket mSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int res = 0;

	res = mSocket.Bind(ip, port);
	if(0 != res)
		return res;

	res = mSocket.Listen();
	if(0 != res)
		return res;

	printf("Begin epoll model listen port:%d\n", port);

	m_max_fd = max_fd;

	m_events = new epoll_event[m_max_fd];

	m_epfd = epoll_create(m_max_fd);
	if (0 >= m_epfd)
		return m_epfd;

	res = AddSocket(mSocket);
		
	if(0 != res)
		return res;

	m_ioType = SI_EPOLL;
	return res;
}

int MyEpoll::AddSocket(const MySocket& s)
{
	if (!m_sockets.insert(std::make_pair(s.getSocket(), s)).second)
		return -1;

	epoll_event ep_event;
	ep_event.data.fd = s.getSocket();
	ep_event.events = EPOLLIN | EPOLLET;

	epoll_ctl(m_epfd, EPOLL_CTL_ADD, s.getSocket(), &ep_event);

	return 0;
}


int MyEpoll::AddSocket(uint32 fd)
{
	epoll_event ep_event;
	ep_event.data.fd = fd;
	ep_event.events = EPOLLIN | EPOLLET;

	epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ep_event);

	return 0;
}


int MyEpoll::DelSocket(uint32 fd)
{
	epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);

	reutrn 0;
}


void MyEpoll::WaitEvent()
{
	int cnt = epoll_wait(m_epfd, m_events, m_max_fd, 1);
	if (0 > cnt)
		return;

	IOEvent newEvent;

	for (int i = 0; i < cnt; ++i)
	{
		
		if (m_events[i].events & EPOLLIN)
			newEvent.sock_event = SE_READ;

		else if (m_events[i].events & EPOLLOUT)
			newEvent.sock_event = SE_WRITE;

		else if (m_events[i].events & EPOLLERR)
			newEvent.sock_event = SE_EXCEPT;

		else
		{
			printf("unexpected event:%d\n", m_events[i].events);
			continue;
		}

		newEvent.fd = m_events[i].data.fd;
		addIOEvent(newEvent);
	}
}

void MyEpoll::HandleEvent(const IOEvent& ioEvent)
{
	std::map<uint32, MySocket>::iterator it = m_sockets.find(ioEvent.fd);

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
			MySocket new_socket;
            int acceptNum = 0;
            while(true)
            {
                int ret = it->second.Accept(new_socket);
                if (0 < ret)
                    AddSocket(new_socket);

                else if(0 > ret && EAGAIN == errno)
                {
                    printf("accept connect:%d\n", acceptNum);
                    break;
                }                

                else
                {
                    printf("accept failed!\n");
                    break;
                }
                ++ acceptNum;
            }
		}

		else
		{
            int n = 0;
            int totalLen = 0;
            int len = 0;
            ioctl(it->second.getSocket(), FIONREAD, &n);

            totalLen = it->second.Recv(m_recv_buf, MAX_RECV_BUF_SIZE);
            if(0 >= totalLen) 
            {
                it->second.Close();
                delSocket(it->first);
            }           

            while(n - totalLen > 0)
            {
                len = it->second.Recv(m_recv_buf, MAX_RECV_BUF_SIZE);
			    //if (0 >= it->second.Recv(m_recv_buf, MAX_RECV_BUF_SIZE))
			   if(0 >= len)
			{
				it->second.Close();
				delSocket(it->first);
                break;
			}
                totalLen += len;
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

void MyEpoll::delSocket(const uint32 fd)
{
	m_sockets.erase(fd);
	epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
}

#endif // !_WIN32
