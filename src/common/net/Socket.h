/************C++ Header File****************
#
#	Filename: Socket.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:18:05
#	Last Modified: 2018-08-10 10:18:05
*******************************************/

#pragma once

#include "stdafx.h"
#include "common.h"

#define FD_INVALID 0

#define MAX_IP_SIZE	32

namespace chaos
{
	class Socket : public NonCopyable
	{
	public:
		~Socket();

		explicit Socket(socket_t fd) :
			m_fd(fd)
		{}

		Socket(int af, int type, int protocol);

	public:
		int Bind(const sockaddr* sa, int salen);

		int Listen(int backlog);

		socket_t Accept();

		int Connect(int af, const char* strIP, const int nPort);

		int Connect(sockaddr* sa, int salen);

		int Close();

		int Recv(char* buf, const int size);

		int Recv(IOVEC_TYPE* iov, int iovcnt);

		int Send(const char* buf, const int size);

		int Send(IOVEC_TYPE* iov, int iovcnt);
		
		socket_t GetFd() const { return m_fd; }

		//缓冲区中的待接收数据大小
		socket_unread_t GetUnreadByte() const;

		//设置非阻塞
		int SetNonBlock();

		int CloseOnExec();

	private:
		socket_t m_fd;
	};



#ifdef _WIN32

	class WsaData
	{
	public:
		static WsaData& Instance()
		{
			static WsaData inst_;
			return inst_;
		}


		~WsaData()
		{
			WSACleanup();
		}

	private:
		WsaData()
		{
			if (0 != WSAStartup(MAKEWORD(2, 2), &m_wsa))
			{
				printf("initialize windows socket failed!\n");
				exit(0);
			}
		}

	private:
		WSADATA m_wsa;
	};

#endif // _WIN32
}
