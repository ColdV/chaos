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

#ifdef _WIN32
	WsaData g_wsa = WsaData::Instance();
#endif // _WIN32


MySocket::MySocket()
{
	initSokcet();
}

MySocket::MySocket(int af, int type, int protocol)
{
	initSokcet();

	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}


MySocket::~MySocket()
{
    //Close();
}


int MySocket::initSokcet()
{
	m_type = SKT_INVALID;
	m_fd = 0;
	memset(m_ip, 0, sizeof(m_ip));
	m_port = 0;

	return 0;
}

uint32 MySocket::Socket()
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
    //靠靠縧isten socket
    int oldFlags = fcntl(m_fd, F_GETFL);
    fcntl(m_fd, F_SETFL, oldFlags | O_NONBLOCK);

    int res = listen(m_fd, 5);
	if(0 != res) //if (0 != listen(m_fd, 5))
		return res;

	m_type = SKT_LISTEN;

	return res;
}

int MySocket::Accept(MySocket& mySocket)
{
	//printf("开始接受新的连接!\n");
	if (SKT_LISTEN != m_type)
		return -1;

    int n = 0;
    if(ioctl(m_fd, FIONREAD, &n) >= 0)
        printf("listen socket[%d] ready accept msg:%d\n", m_fd, n);

	sockaddr_in sockAddr;
	socklen_t len = sizeof(sockAddr);
	memset(&sockAddr, 0, sizeof(sockAddr));
	uint32 nfd = accept(m_fd, (sockaddr*)&sockAddr, &len);
	if (0 >= nfd)
		return nfd;

    if(ioctl(m_fd, FIONREAD, &n) >= 0)
        printf("listen socket[%d] left accept msg:%d\n", m_fd, n);


	inet_ntop(AF_INET, &sockAddr.sin_addr, mySocket.getIP(), MAX_IP_SIZE);
	mySocket.setPort(ntohs(sockAddr.sin_port));
	mySocket.setSocket(nfd);
	mySocket.setType(SKT_SERVER);

	printf("accept from ip:%s, port:%d, new socket:%d\n", mySocket.getIP(), mySocket.getPort(), nfd);

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
	//printf("开始接受数据！\n");
	int n = 0;
    if(ioctl(m_fd, FIONREAD, &n) >= 0)
        printf("socket[%d] ready recv msg len:%d\n", m_fd, n);

	int len = recv(m_fd, buf, size, 0); //MSG_PEEK
    printf("socket[%d] already recv msg len:%d\n", m_fd, len);

	if (0 >= len)
	{
		printf("recv client socket[%d] close msg!\n", m_fd);
		//Close();
		return -1;
	}
    
    if(8 == m_fd)
    {
        //sleep(5);
    }

	printf("recv data[%s],from ip:%s,port:%d, fd:%d\n", buf, m_ip, m_port, m_fd);

	
	len = send(m_fd, buf, strlen(buf), 0);
	printf("send data to socket[%d] len:%d\n", m_fd, len);

	//memset(m_recv_buf, 0, sizeof(m_recv_buf));
	
	return len;
}

int MySocket::Send(const char* buf, const int size)
{
	int len = send(m_fd, buf, size, 0);
	printf("send data to socket[%d] len:%d\n", m_fd, len);

	return len;
}
