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
#include <stack>
#include <list>
#include "noncopyable.h"

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024

#endif // !FD_SETSIZE

#ifdef _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#endif  //_WIN32

typedef unsigned char				byte; 
typedef char						int8;
typedef byte						uint8;
typedef short						int16;
typedef unsigned short				uint16;
typedef	int							int32;
typedef unsigned int				uint32;
typedef long long int				int64;
typedef unsigned long long int		uint64;
typedef uint32						timer_id;

#ifdef WIN32
typedef SOCKET socket_t;
typedef uint32 thread_t;
typedef HANDLE sem_t;
typedef HANDLE mutex_t;
typedef HANDLE cond_t;
typedef DWORD sem_wait_ret;
typedef DWORD mutex_lock_ret;

#else
typedef int socket_t;
typedef pthread_t thread_t;
typedef int sem_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;
typedef int sem_wait_ret;
typedef int mutex_lock_ret;

#endif // WIN32


const int MAX_INT = 0x7FFFFFFF;



