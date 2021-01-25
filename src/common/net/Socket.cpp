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
	static WsaData& g_wsa = WsaData::Instance();
#endif // _WIN32


	Socket::Socket(int af, int type, int protocol)
	{
		m_fd = socket(af, type, protocol);
		//eventfd()
	}


	Socket::~Socket()
	{
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

		return ret;
	}


	socket_t Socket::Accept()
	{
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


	int Socket::Connect(int af, const char* strIP, const int nPort)
	{
		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));

		sockAddr.sin_family = af;
		sockAddr.sin_port = htons(nPort);
		inet_pton(af, strIP, &sockAddr.sin_addr);

		return Connect((sockaddr*)&sockAddr, sizeof(sockAddr));
	}


	int Socket::Connect(sockaddr* sa, int salen)
	{
		return connect(m_fd, sa, salen);
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

		return len;
	}


	int Socket::Recv(IOVEC_TYPE* iov, int iovcnt)
	{
		int transBytes = 0;
#ifdef _WIN32
		DWORD readBytes = 0;
		DWORD flags = 0;
		if (WSARecv(m_fd, iov, iovcnt, &readBytes, &flags, NULL, NULL))
		{
			if (WSAGetLastError() != WSAECONNABORTED)
				transBytes = -1;
		}
		else
		{
			transBytes = readBytes;
		}
#else
		transBytes = readv(m_fd, iov, iovcnt);
#endif // _WIN32

		return transBytes;
	}


	int Socket::Send(const char* buf, const int size)
	{
		int len = send(m_fd, buf, size, 0);

		return len;
	}


	int Socket::Send(IOVEC_TYPE* iov, int iovcnt)
	{
		int transBytes = 0;
#ifdef _WIN32
		DWORD sendBytes = 0;
		if (WSASend(m_fd, iov, iovcnt, &sendBytes, 0, NULL, NULL))
			transBytes = -1;
		else
			transBytes = sendBytes;	
#else
		transBytes = writev(m_fd, iov, iovcnt);
#endif // _WIN32

		return transBytes;
	}


	int Socket::GetUnreadByte() const
	{
#ifdef _WIN32
		u_long n = 0;
		if (ioctlsocket(m_fd, FIONREAD, &n) < 0)
		{
			printf("call ioctlsocket failed! err:%d\n", WSAGetLastError());
			return 0;
		}

		return (int)n;
#else
		int n = 0;
		if (ioctl(m_fd, FIONREAD, &n) < 0)
		{
			printf("call ioctlsocket failed!\n");
			return -1;
		}

		return n;
#endif // _WIN32
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