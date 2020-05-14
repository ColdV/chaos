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
#include "../../common/common.h"

namespace NetFrame
{

#ifdef _WIN32
	static WsaData g_wsa = WsaData::Instance();
#endif // _WIN32


	Socket::Socket(socket_t fd):
		m_type(SKT_INVALID),
		m_fd(fd),
		m_port(0),
		m_ip{0}
	{
	}

	Socket::Socket(int af, int type, int protocol):
		m_type(SKT_INVALID),
		m_port(0),
		m_ip{ 0 }
	{
		/*Init();*/
		m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}


	Socket::~Socket()
	{
		//Close();
	}


	int Socket::Init()
	{
		m_type = SKT_INVALID;
		memset(m_ip, 0, sizeof(m_ip));
		m_port = 0;

		return 0;
	}


	int Socket::Bind(const char* strIP, const int nPort)
	{
		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));

		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(nPort);
		//inet_pton(AF_INET, strIP, &sockAddr.sin_addr);

		if (0 != bind(m_fd, (sockaddr*)&sockAddr, sizeof(sockaddr)))
			return -1;

		strncpy_safe(m_ip, sizeof(m_ip), strIP, sizeof(m_ip));
		m_port = nPort;

		return 0;
	}

	int Socket::Listen()
	{
		//靠靠縧isten socket
#ifdef _WIN32
		u_long mode = 1;
		if (SOCKET_ERROR == ioctlsocket(m_fd, FIONBIO, &mode))
		{
			printf("set socket nonblock failed!\n");
			return -1;
		}

#else
		int oldFlags = fcntl(m_fd, F_GETFL);
		fcntl(m_fd, F_SETFL, oldFlags | O_NONBLOCK);

#endif // _WIN32

		int res = listen(m_fd, 128);
		if (0 != res) //if (0 != listen(m_fd, 5))
			return res;

		m_type = SKT_LISTEN;

		return res;
	}

	socket_t Socket::Accept(/*Socket& mySocket*/)
	{
		//printf("开始接受新的连接!\n");
		if (SKT_LISTEN != m_type)
			return -1;

		sockaddr_in sockAddr;
		socklen_t len = sizeof(sockAddr);
		memset(&sockAddr, 0, sizeof(sockAddr));

#ifdef _WIN32
		socket_t fd = accept(m_fd, (sockaddr*)&sockAddr, &len);
#else
		uint32 nfd = accept4(m_fd, (sockaddr*)&sockAddr, &len, 2048);
#endif
		if (0 >= fd)
			return fd;

		//inet_ntop(AF_INET, &sockAddr.sin_addr, mySocket.getIP(), MAX_IP_SIZE);
		//mySocket.setPort(ntohs(sockAddr.sin_port));
		//mySocket.setSocket(nfd);
		//mySocket.setType(SKT_SERVER);

		//printf("accept from ip:%s, port:%llu, new socket:%llu\n", mySocket.getIP(), mySocket.getPort(), nfd);

		return fd;
	}


	Socket* Socket::Accept2()
	{
		return new Socket(Accept());
	}


	int Socket::Connect(const char* strIP, const int nPort)
	{
		if (0 != m_type)
			return -1;

		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));

		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = htons(nPort);
		inet_pton(AF_INET, strIP, &sockAddr.sin_addr);

		int res = connect(m_fd, (sockaddr*)&sockAddr, sizeof(sockAddr));

		if (0 != res)
			return -1;

		m_type = SKT_CLIENT;

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
		//printf("开始接受数据！\n");

#ifdef _WIN32
		unsigned long n = 0;
		if (ioctlsocket(m_fd, FIONREAD, &n) < 0)
			printf("socket[%llu] ready recv msg len:%lu\n", m_fd, n);

#else
		int n = 0;
		if (ioctl(m_fd, FIONREAD, &n) >= 0)
			printf("socket[%d] ready recv msg len:%llu\n", m_fd, n);

#endif // _WIN32

		int len = recv(m_fd, buf, size, 0); //MSG_PEEK
		printf("socket[%llu] already recv msg len:%d\n", m_fd, len);

		if (0 >= len)
		{
			printf("recv client socket[%llu] close msg!\n", m_fd);
			//Close();
			return -1;
		}

		if (8 == m_fd)
		{
			//sleep(5);
		}

		printf("recv data[%s],from ip:%s,port:%d, fd:%llu\n", buf, m_ip, m_port, m_fd);


		len = send(m_fd, buf, strlen(buf), 0);
		printf("send data to socket[%llu] len:%d\n", m_fd, len);

		//memset(m_recv_buf, 0, sizeof(m_recv_buf));

		return len;
	}

	int Socket::Send(const char* buf, const int size)
	{
		int len = send(m_fd, buf, size, 0);
		printf("send data to socket[%u] len:%d\n", m_fd, len);

		return len;
	}

}