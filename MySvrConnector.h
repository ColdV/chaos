/************C++ Header File****************
#
#	Filename: MySvrConnector.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-07-31 10:31:29
#	Last Modified: 2018-07-31 10:31:29
*******************************************/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string.h>

#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#endif  //_WIN32














struct socket_info
{
	unsigned int socket;
	sockaddr_in socket_addr;	
};

class MySvrConnector
{
public:
	explicit MySvrConnector(int nMaxConnect);
	~MySvrConnector();

	void run();
	
private:
	int Socket();
	int Bind();
	int Listen();
	int Accept(/*unsigned int nfd, const sockaddr_in* pSockaddr*/);
	int Recv();
	int Send();

private:
	unsigned int m_listen_socket;	
	sockaddr_in m_sockaddr;
	unsigned int m_max_connect;		//最大连接数
	unsigned int m_cur_maxfds;		//当前连接的数量
	
	fd_set m_rfds;	//读fd

	char m_recv_buf[1024 * 10];
	unsigned int m_recv_bytes;		

	char m_send_buf[1024 * 10];
	unsigned int m_send_bytes;

	std::map<unsigned int, socket_info> client_socket_map;
};
