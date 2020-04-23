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

#include "../common/stdafx.h"
#include "../common/common.h"

#define FD_INVALID 0

enum SocketType
{
	SKT_INVALID = 0,
	SKT_LISTEN,			//监听套接字(Listen)
	SKT_CLIENT,			//客户端套接字(Connect)
	SKT_SERVER,			//服务端套接字(Accept)
};

#define MAX_IP_SIZE	32

namespace NetFrame
{
	class Socket
	{
	public:
		explicit Socket(socket_t fd);
		Socket(int af, int type, int protocol);
		~Socket();

	public:
		int Bind(const char* strIP, const int nPort);

		int Listen();

		int Accept(/*Socket& sock*/);

		int Connect(const char* strIP, const int nPort);

		int Close();

		int Recv(char* buf, const int size);

		int Send(const char* buf, const int size);

	public:
		socket_t GetFd() const { return m_fd; }

		uint32 GetFdType() const { return m_type; }

		const char* GetIP() const { return m_ip; }

		uint32 GetPort() const { return m_port; }

	private:
		int Init();

	private:
		SocketType m_type;
		socket_t m_fd;
		int m_port;
		char m_ip[MAX_IP_SIZE];
	};



#ifdef _WIN32

	class WsaData
	{
	public:
		static WsaData& Instance()
		{
			static WsaData wsa_;
			return wsa_;
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
