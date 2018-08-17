/************C++ Source File****************
#
#	Filename: MySocket.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:18:21
#	Last Modified: 2018-08-10 10:18:21
*******************************************/

#include "MySocket.h"
#include "../common/common.h"

MySocket::MySocket()
{
/*
	m_type = SKT_INVALID;
	m_fd = 0;
	memset(m_ip, 0, sizeof(m_ip));
	m_port = 0;

#ifdef _WIN32
	static WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		printf("initialize windows socket failed!\n");
		exit(0);
	}
#endif	// _WIN32
*/
	initSokcet();
}

MySocket::MySocket(int af, int type, int protocol)
{
	initSokcet();

	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}


MySocket::~MySocket()
{

}


int MySocket::initSokcet()
{
	m_type = SKT_INVALID;
	m_fd = 0;
	memset(m_ip, 0, sizeof(m_ip));
	m_port = 0;

	return 0;
}

int MySocket::Socket()
{
	if (FD_INVALID != m_fd)
		return m_fd;

	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	return m_fd;
}

int MySocket::Bind(const char* strIP, const int nPort)
{
	sockaddr_in sockAddr;

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(nPort);
	inet_pton(AF_INET, strIP, &sockAddr.sin_addr);

	if (0 != bind(m_fd, (sockaddr*)&sockAddr, sizeof(sockaddr)))
		return -1;

	strncpy_safe(m_ip, sizeof(m_ip), strIP, sizeof(m_ip));
	m_port = nPort;

	return 0;
}

int MySocket::Listen()
{
	if (0 != listen(m_fd, 5))
		return -1;

	m_type = SKT_LISTEN;

	return 0;
}

int MySocket::Accept()
{
	if (SKT_LISTEN != m_type)
		return -1;

	sockaddr_in sockAddr;
	socklen_t len = sizeof(sockAddr);
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nfd = accept(m_fd, (sockaddr*)&sockAddr, &len);
	if (0 >= nfd)
		return nfd;

	return nfd;
}

int MySocket::Connect(const char* strIP, const int nPort)
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

int MySocket::Close()
{
#ifdef _WIN32
	return closesocket(m_fd);

#else
	return close(m_fd);

#endif // _WIN32
}

int MySocket::Recv(char* buf, const int size)
{
	int len = recv(m_fd, buf, size, 0);
	if (0 >= len)
	{
		printf("recv client socket[%d] close msg!\n", m_fd);
		Close();
		return -1;
	}
	printf("recv data[%s],from ip:%s,port:%d, fd:%d\n", buf, m_ip, m_port, m_fd);

	/*
	n = send(m_fd, m_recv_buf, strlen(m_recv_buf), 0);
	printf("send data to socket[%d] len:%d\n", nfd, n);

	memset(m_recv_buf, 0, sizeof(m_recv_buf));
	*/
	return len;
}

int MySocket::Send(const char* buf, const int size)
{
	int len = send(m_fd, buf, size, 0);
	printf("send data to socket[%d] len:%d\n", m_fd, len);

	return len;
}