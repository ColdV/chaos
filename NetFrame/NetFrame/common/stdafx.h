/************C++ Header File****************
#
#	Filename: stdafx.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:18:43
#	Last Modified: 2018-08-10 10:18:43
*******************************************/

#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <queue>
#include <string.h>

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024

#endif // !FD_SETSIZE

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

typedef unsigned char	byte; 
typedef short			int16;
typedef unsigned short	uint16;
typedef	int				int32;
typedef unsigned int	uint32;
typedef long long int	int64;
typedef unsigned long long int uint64;



