/************C++ Header File****************
#
#	Filename: MySocket.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:18:05
#	Last Modified: 2018-08-10 10:18:05
*******************************************/

#pragma once

#include "stdafx.h"

#define FD_INVALID 0

enum SocketType
{
	SKT_INVALID = 0,
	SKT_LISTEN,			//监听套接字(Listen)
	SKT_CLIENT,			//客户端套接字(Connect)
	SKT_SERVER,			//服务端套接字(Accept)
};

class MySocket
{
public:
	MySocket();
	~MySocket();
	int Socket();
	int Bind(const char* strIP, const int nPort);
	int Listen();
	int Accept();
	int Connect(const char* strIP, const int nPort);
	int Close();
	int Recv(char* buf, const int size);
	int Send(const char* buf, const int size);

public:
	uint32 getSocket() const { return m_fd; }
	uint32 getType() const { return m_type; }
	const char* getIP() const { return m_ip; }
	uint32 getPory() const { return m_port; }

private:
	SocketType m_type;
	uint32 m_fd;
	char m_ip[32];
	uint32 m_port;
};