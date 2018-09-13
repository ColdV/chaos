/************C++ Source File****************
#
#	Filename: test.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-02 16:28:44
#	Last Modified: 2018-08-02 16:28:44
*******************************************/


#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_FD	10
#define IP	"192.168.2.109"

int main()
{

	fd_set fds;
	FD_ZERO(&fds);
	int nfds[MAX_FD] = {0};
	char recvBuf[1024] = {0};
	int maxFd = 0;

	for(int i = 0; i < sizeof(nfds) / sizeof(nfds[0]); ++i)
	{
		nfds[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(0 >= nfds[i])
		{
			printf("create socket failed! fds[%d]:%d\n", i, nfds[i]);
			continue;
		}
	}

	sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(6666);
	inet_pton(AF_INET, IP, (void*)&clientAddr.sin_addr);

	for(int i = 0; i < sizeof(nfds) / sizeof(nfds[0]); ++i)
	{
		int res = connect(nfds[i], (sockaddr*)&clientAddr, sizeof(clientAddr));
		if(0 != res)
		{
			printf("connect failed! fds[%d]:%d\n", i, nfds[i]);
			continue;
		}
		FD_SET(nfds[i], &fds);
		if(nfds[i] > maxFd)
			maxFd = nfds[i];
		printf("connect sucess fd:%d\n", nfds[i]);
	}

	char send_buf[128] = {0};

	
	for(int i = 0; i < sizeof(nfds) / sizeof(nfds[0]); ++i)
	{
		sprintf(send_buf, "%si am socket[%d]", "hello world!", nfds[i]);
		int res = send(nfds[i], send_buf, strlen(send_buf), 0);
		printf("socket[%d] send msg len:%d\n",nfds[i],  res);
		memset(send_buf, 0, sizeof(send_buf));
	}
	
	
	while(true)
	{
		fd_set fd_copy = fds;

		//printf("inter main loop socket[%d], fd_set[%d], src_fd[%d]\n", nfds[0], fd_copy.fds_bits[0], fds.fds_bits[0]);
		int n = select(maxFd + 1, &fd_copy, NULL, NULL, NULL);


		if(0 > n)
		{
			printf("select call error!\n", n);
			return 0;
		}	

/*
		if(0 == n)
		{
			printf("socket[%d], fd_copy[%d]\n", nfds[0], fd_copy);
			return 0;
		}
		
		if(0 > n)
		{
			printf("大于0\n");
		}
*/
		for(int i = 0; i < MAX_FD; ++i)
		{
//			printf("socket[%d], fd_copy[%d]\n", nfds[i], fd_copy);
			if(FD_ISSET(nfds[i], &fd_copy))
			{
				int len = recv(nfds[i], recvBuf, sizeof(recvBuf), 0);
				if(0 < len)
				{
					printf("socket[%d] recv msg:%s\n", nfds[i], recvBuf);
				}
				else
				{
					if(-1 == len)
						perror("recv -1:");
					printf("recv fail! code:%d, socket:%d\n", len, nfds[i]);

					FD_CLR(nfds[i], &fds);
					close(nfds[i]);
				}

				memset(recvBuf, 0, sizeof(recvBuf));
			}
		}
	}
	
	/*
	for(int i = 0; i < sizeof(nfds) / sizeof(nfds[0]); ++i)
	{
		close(nfds[i]);
	}
	*/
	return 0;
}
