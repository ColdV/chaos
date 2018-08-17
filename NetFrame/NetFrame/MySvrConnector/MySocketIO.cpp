/************C++ Source File****************
#
#	Filename: MySocketIO.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:02
#	Last Modified: 2018-08-11 17:40:02
*******************************************/


#include "MySocketIO.h"

MySocketIO::MySocketIO()
{
	m_sockets.clear();
	m_max_socket = 0;
	memset(m_recv_buf, 0, MAX_RECV_BUF_SIZE);

#ifdef _WIN32

	static WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		printf("initialize windows socket failed!\n");
		exit(0);
	}

#endif // _WIN32

}

MySocketIO::~MySocketIO()
{
#ifdef _WIN32
	WSACleanup();
#endif // _WIN32
}