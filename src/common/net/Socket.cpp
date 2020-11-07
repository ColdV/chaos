/************C++ Source File****************
#
#	Filename: Socket.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:18:21
#	Last Modified: 2018-08-10 10:18:21
*******************************************/

#include "Socket.h"
#include "common.h"

namespace chaos
{

#ifdef _WIN32
	static WsaData g_wsa = WsaData::Instance();
#endif // _WIN32


	Socket::Socket(int af, int type, int protocol)
	{
		m_fd = socket(af, type, protocol);
	}


	Socket::~Socket()
	{
		printf("close socket:%llu\n", m_fd);
		Close();
	}


	int Socket::Bind(const sockaddr* sa, int salen)
	{
		if (!sa)
			return -1;

		if (0 != bind(m_fd, sa, salen))
			return -1;

		return 0;
	}


	int Socket::Listen(int backlog)
	{
		int ret = listen(m_fd, backlog);
		if (0 != ret)
			return ret;

		return ret;
	}


	socket_t Socket::Accept()
	{
		//printf("开始接受新的连接!\n");

		sockaddr_in sockAddr;
		socklen_t len = sizeof(sockAddr);
		memset(&sockAddr, 0, sizeof(sockAddr));

#ifdef _WIN32
		socket_t connfd = accept(m_fd, (sockaddr*)&sockAddr, &len);
#else
		socket_t connfd = accept4(m_fd, (sockaddr*)&sockAddr, &len, SOCK_NONBLOCK);
#endif
		return connfd;
	}


	int Socket::Connect(const char* strIP, const int nPort)
	{
		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));

		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(nPort);
		inet_pton(AF_INET, strIP, &sockAddr.sin_addr);

		int res = connect(m_fd, (sockaddr*)&sockAddr, sizeof(sockAddr));

		if (0 != res)
			return -1;

		return 0;
	}

	int Socket::Close()
	{
#ifdef _WIN32
		return closesocket(m_fd);

#else
		return close(m_fd);

#endif // _WIN32
	}


	int Socket::Recv(char* buf, const int size)
	{
		int len = recv(m_fd, buf, size, 0);

		if (0 >= len)
		{
			printf("recv client socket[%llu] close msg!\n", m_fd);
			return -1;
		}

		return len;
	}

	int Socket::Send(const char* buf, const int size)
	{
		int len = send(m_fd, buf, size, 0);

		return len;
	}


	socket_unread_t Socket::GetUnreadByte() const
	{
		socket_unread_t n = 0;
#ifdef _WIN32
		if (ioctlsocket(m_fd, FIONREAD, &n) < 0)
		{
			printf("call ioctlsocket failed! err:%d\n", WSAGetLastError());
			return 0;
		}
		/*else
		{
			printf("socket[%llu] ready recv msg len:%lu\n", m_fd, n);
		}*/

#else
		if (ioctl(m_fd, FIONREAD, &n) < 0)
		{
			printf("call ioctlsocket failed!\n");
			return 0;
		}
		/*else
		{
			printf("socket[%d] ready recv msg len:%llu\n", m_fd, n);	
		}*/

#endif // _WIN32

		return n;
	}


	int Socket::SetNonBlock()
	{
#ifdef _WIN32
		u_long mode = 1;
		int ret = ioctlsocket(m_fd, FIONBIO, &mode);

#else
		int oldFlags = fcntl(m_fd, F_GETFL);
		int ret = fcntl(m_fd, F_SETFL, oldFlags | O_NONBLOCK);

#endif // _WIN32

		return ret;
	}


	int Socket::CloseOnExec()
	{
#ifndef _WIN32
		if (fcntl(m_fd, F_SETFD, FD_CLOEXEC) == -1) 
		{
			return -1;
		}
#endif
		return 0;
	}

}