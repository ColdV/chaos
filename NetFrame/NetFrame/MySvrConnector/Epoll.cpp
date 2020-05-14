/************C++ Source File****************
#
#	Filename: Epoll.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:46
#	Last Modified: 2018-08-10 10:12:46
*******************************************/

#ifndef _WIN32

#include "Epoll.h"


namespace NetFrame
{ 

	Epoll& Epoll::Instance()
	{
		static Epoll s_inst;
		return s_inst;
	}

	Epoll::Epoll()
	{
	}


	int Epoll::InitIO(/*const char* ip, int port, uint32 max_fd*/)
	{
		//Socket mSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//int res = 0;

		//res = mSocket.Bind(ip, port);
		//if(0 != res)
		//	return res;

		//res = mSocket.Listen();
		//if(0 != res)
		//	return res;

		//printf("Begin epoll model listen port:%d\n", port);

		//m_max_fd = max_fd;

		m_events = new epoll_event[MAX_FD];

		m_epfd = epoll_create(MAX_FD);
		if (0 >= m_epfd)
			return m_epfd;

		//res = AddSocket(mSocket);
		//	
		//if(0 != res)
		//	return res;

		//m_ioType = SI_EPOLL;
		return 0;
	}

	//int Epoll::AddSocket(const Socket& s)
	//{
	//	if (!m_sockets.insert(std::make_pair(s.getSocket(), s)).second)
	//		return -1;
	//
	//	epoll_event ep_event;
	//	ep_event.data.fd = s.GetFd();
	//	ep_event.events = EPOLLIN | EPOLLET;
	//
	//	epoll_ctl(m_epfd, EPOLL_CTL_ADD, s.GetFd(), &ep_event);
	//
	//	return 0;
	//}
	//
	//
	//int Epoll::AddSocket(uint32 fd)
	//{
	//	epoll_event ep_event;
	//	ep_event.data.fd = fd;
	//	ep_event.events = EPOLLIN | EPOLLET;
	//
	//	epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ep_event);
	//
	//	return 0;
	//}
	//
	//
	//int Epoll::DelSocket(uint32 fd)
	//{
	//	epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	//
	//	return 0;
	//}


	//void Epoll::WaitEvent()
	void Epoll::Launch()
	{
		int cnt = epoll_wait(m_epfd, m_events, MAX_FD, 1);
		if (0 > cnt)
			return -1;

		//IOEvent newEvent;
		FdEvent fdEv;

		for (int i = 0; i < cnt; ++i)
		{
		
			if (m_events[i].events & EPOLLIN)
				fdEv.ev = EV_IOREAD;

			else if (m_events[i].events & EPOLLOUT)
				fdEv.ev = EV_IOWRITE;

			else if (m_events[i].events & EPOLLERR)
				fdEv.ev = EV_IOEXCEPT;

			else
			{

				printf("unexpected event:%d\n", m_events[i].events);
				continue;
			}

			fdEv.fd = m_events[i].data.fd;
			/*addIOEvent(newEvent);*/
			PushActiveFd(fdEv);
		}

		return 0;
	}


	void Epoll::RegistFd(socket_t fd, short ev)
	{
		epoll_event epEv;
		epEv.data.fd = fd;
		epEv.events = EPOLLIN | EPOLLET;
		
		epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &epEv);
	}


	void Epoll::CancelFd(socket_t fd, short ev)
	{
		epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	}


	//void Epoll::HandleEvent(const IOEvent& ioEvent)
	//{
	//	std::map<uint32, Socket>::iterator it = m_sockets.find(ioEvent.fd);
	//
	//	if (it == m_sockets.end())
	//		return;
	//
	//	switch (ioEvent.sock_event)
	//	{
	//	default:
	//		break;
	//
	//	case SE_READ:
	//	{
	//		if (it->second.getType() == SKT_LISTEN)
	//		{
	//			Socket new_socket;
	//            int acceptNum = 0;
	//            while(true)
	//            {
	//                int ret = it->second.Accept(new_socket);
	//                if (0 < ret)
	//                    AddSocket(new_socket);
	//
	//                else if(0 > ret && EAGAIN == errno)
	//                {
	//                    printf("accept connect:%d\n", acceptNum);
	//                    break;
	//                }                
	//
	//                else
	//                {
	//                    printf("accept failed!\n");
	//                    break;
	//                }
	//                ++ acceptNum;
	//            }
	//		}
	//
	//		else
	//		{
	//            int n = 0;
	//            int totalLen = 0;
	//            int len = 0;
	//            ioctl(it->second.getSocket(), FIONREAD, &n);
	//
	//            totalLen = it->second.Recv(m_recv_buf, MAX_RECV_BUF_SIZE);
	//            if(0 >= totalLen) 
	//            {
	//                it->second.Close();
	//                delSocket(it->first);
	//            }           
	//
	//            while(n - totalLen > 0)
	//            {
	//                len = it->second.Recv(m_recv_buf, MAX_RECV_BUF_SIZE);
	//			    //if (0 >= it->second.Recv(m_recv_buf, MAX_RECV_BUF_SIZE))
	//			   if(0 >= len)
	//			{
	//				it->second.Close();
	//				delSocket(it->first);
	//                break;
	//			}
	//                totalLen += len;
	//            }
	//		}
	//	}
	//	break;
	//
	//	case SE_WRITE:
	//		break;
	//
	//	case SE_EXCEPT:
	//		printf("socket[%d] except\n", it->first);
	//		delSocket(it->first);
	//		//it->second.Close();
	//		break;
	//
	//	}
	//
	//	DelIOEvent();
	//}

	//void Epoll::delSocket(const uint32 fd)
	//{
	//	m_sockets.erase(fd);
	//	epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	//}

}

#endif // !_WIN32
