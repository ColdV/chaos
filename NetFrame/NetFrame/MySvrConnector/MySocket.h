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

class MySocket
{
public:
	MySocket();
	MySocket(int af, int type, int protocol);
	virtual ~MySocket();

public:
	uint32 Socket();
	int Bind(const char* strIP, const int nPort);
	int Listen();
	int Accept(MySocket& mySokcet);
	int Connect(const char* strIP, const int nPort);
	int Close();
	int Recv(char* buf, const int size);
	int Send(const char* buf, const int size);

public:
	uint32 getSocket() const { return m_fd; }
	void setSocket(uint32 fd) { m_fd = fd; }

	uint32 getType() const { return m_type; }
	void setType(SocketType type) { m_type = type; }

	const char* getIP() const { return m_ip; }
	char* getIP() { return m_ip; }
	void setIP(char* ip) { strncpy_safe(m_ip, sizeof(m_ip), ip, strlen(ip)); }

	uint32 getPort() const { return m_port; }
	void setPort(uint32 port) { m_port = port; }

private:
	int initSokcet();

private:
	SocketType m_type;
	uint32 m_fd;
	char m_ip[MAX_IP_SIZE];
	uint32 m_port;
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
