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

#include "../../common/stdafx.h"
#include "../../common/common.h"

#define FD_INVALID 0

//enum SocketType
//{
//	SKT_INVALID = 0,
//	SKT_LISTEN = 1,				//监听套接字(Listen)
//	SKT_CLIENT = 1 << 1,		//客户端套接字(Connect)
//	SKT_SERVER = 1 << 2,		//服务端套接字(Accept)
//	SKT_CONNING = 1 << 3,		//已经连接的套接字
//};

#define MAX_IP_SIZE	32

namespace NetFrame
{
	class Socket : public NonCopyable
	{
	public:
		//explicit Socket(socket_t fd, bool isBlock = false);			//不需要type了
		Socket(socket_t fd, sockaddr_in* addr, bool isBlock);
		Socket(int af, int type, int protocol, bool isBlock);
		~Socket();

	public:
		int Bind(const char* strIP, const int nPort);

		int Listen();

		Socket* Accept();

		//Socket* Accept2();

		int Connect(const char* strIP, const int nPort);

		int Close();

		int Recv(char* buf, const int size);

		int Send(const char* buf, const int size);

	public:
		socket_t GetFd() const { return m_fd; }

		//uint32 GetFdType() const { return m_type; }

		const char* GetIP() const { return m_ip; }

		uint32 GetPort() const { return m_port; }

		bool Block() const { return m_isBlock; }

	private:
		Socket(const Socket&) {}
		//Socket(socket_t fd, uint32 type, int port, char* ip, bool isBlock = false);
		/*int Init();*/

	private:
		socket_t m_fd;
		bool m_isBlock;
		//uint32 m_type;
		int m_port;
		char m_ip[MAX_IP_SIZE];
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
