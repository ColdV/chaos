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
#include <time.h>
#include <vector>
#ifdef WIN32

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#endif

#define MAX_FD	1000
#define IP	"192.168.0.101"
#define PORT	3307

void CloseSocket(int nfd)
{
#ifdef _WIN32
	closesocket(nfd);
#else
	close(nfd);
#endif
}


//#if 0
int main()
{
	fd_set fds;
	FD_ZERO(&fds);
	int nfds[MAX_FD] = { 0 };
	char recvBuf[1024] = { 0 };
	int maxFd = 0;
#ifdef _WIN32
	WSADATA wsa_data;
	WSAStartup(0x0202, &wsa_data);
#endif

	for (int i = 0; i < sizeof(nfds) / sizeof(nfds[0]); ++i)
	{
		nfds[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (0 >= nfds[i])
		{
			printf("create socket failed! fds[%d]:%d\n", i, nfds[i]);
			continue;
		}
	}

	sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, (void*)&clientAddr.sin_addr);

	time_t start_time = time(NULL);

	for (int i = 0; i < sizeof(nfds) / sizeof(nfds[0]); ++i)
	{
		int res = connect(nfds[i], (sockaddr*)&clientAddr, sizeof(clientAddr));
		if (0 != res)
		{
			printf("connect failed! fds[%d]:%d\n", nfds[i], res);
			continue;
		}
		FD_SET(nfds[i], &fds);
		if (nfds[i] > maxFd)
			maxFd = nfds[i];
		printf("connect sucess fd:%d\n", nfds[i]);
	}


	time_t conn_time = time(NULL) - start_time;
	printf("conn cost times:%lld\n", conn_time);

	char send_buf[32] = "hello world!";
	//for (int i = 0; i < sizeof(send_buf) - 1 ; ++i)
	//{
	//	send_buf[i] = 'a';
	//}
	
	for (int i = 0; i < sizeof(nfds) / sizeof(nfds[0]); ++i)
	{
		//sprintf_s(send_buf, "%si am socket[%d]", "hello world!", nfds[i]);
		int res = send(nfds[i], send_buf, sizeof(send_buf), 0);
		printf("socket[%d] send msg len:%d\n", nfds[i], res);
		//memset(send_buf, 0, sizeof(send_buf));
	}

	while (true)
	{
#ifdef _WIN32
		if (0 >= fds.fd_count)
		{
			printf("select fds empty!\n");
			break;
		}
#endif // _WIN32
		fd_set fd_copy = fds;
		
		int n = select(maxFd + 1, &fd_copy, NULL, NULL, NULL);

		
		if (0 >= n)
		{
			printf("select call error:%d! res:%d\n", WSAGetLastError(), n);
			perror("select error");
			printf("cost all time:%lld\tconn cost time:%lld\n", time(NULL) - start_time, conn_time);
			break;
		}

		for (int i = 0; i < MAX_FD; ++i)
		{
			//printf("socket[%d], fd_copy[%d]\n", nfds[i], fd_copy);
			if (FD_ISSET(nfds[i], &fd_copy))
			{
				int len = recv(nfds[i], recvBuf, sizeof(recvBuf), 0);
				if (0 < len)
				{
					printf("socket[%d] recv msg:%s\n", nfds[i], recvBuf);
				}
				else
				{
					if (-1 == len)
						perror("recv -1");
					printf("recv fail! code:%d, socket:%d\n", len, nfds[i]);

					FD_CLR(nfds[i], &fds);
					//close(nfds[i]);
					CloseSocket(nfds[i]); 
				}

				memset(recvBuf, 0, sizeof(recvBuf));
			}
		}
		//Sleep(1000);
	}

	while (1)
	{
		Sleep(1000);
	}

	WSACleanup();



	return 0;
}
//#endif



#if 0
int main()
{
	char p[] = "abcd";

	printf("%d\n", sizeof(p));

}
#endif

#if 0
struct Test
{
	char c;
	char a[2];
	char b[5];
	//char a[3];
	//int b;
};

struct RankData
{
	int RoleID;
	int RankValue;
};

class BaseRank
{
public:
	struct RankNode
	{
		RankNode* pLastNode;
		RankData Data;
		RankNode* pNextNode;
	};

	//BaseRank() {}

	BaseRank(int MaxNode) :
		m_pRootNode(NULL),
		m_Root(NULL),
		m_MaxCount(MaxNode),
		m_UseCount(0)
	{
		if (0 >= MaxNode)
			return;

		m_Root = new RankNode[MaxNode];
		if (!m_Root)
			return;
	}

	virtual ~BaseRank() 
	{
		if (m_pRootNode)
			delete[] m_pRootNode;
	}

	bool Insert(RankNode& LastNode, const RankData& Data);

	bool Delete(RankNode& Data);

	const RankData* GetDataByIndex(int Index);

protected:
	RankNode* m_pRootNode;
	RankNode* m_Root;
	int m_UseCount;
	int m_MaxCount;
};


bool BaseRank::Insert(RankNode& LastNode, const RankData& Data)
{
	if (m_UseCount >= m_MaxCount)
		return false;

	RankNode& Node = m_pRootNode[m_UseCount];
	memcpy(&Node.Data, &Data, sizeof(Node.Data));

	if (0 == m_UseCount)
	{
		Node.pLastNode = NULL;
		Node.pNextNode = NULL;
		m_pRootNode = &Node;
	}
	else
	{
		RankNode* pNextNode = LastNode.pNextNode;
		LastNode.pNextNode = &Node;
		Node.pLastNode = &LastNode;

		if (!pNextNode)
		{
			Node.pNextNode = NULL;
		}
		else
		{
			Node.pNextNode = pNextNode;
			pNextNode->pLastNode = &Node;
		}
	}

	++m_UseCount;

	return true;
}


bool BaseRank::Delete(RankNode& Data)
{
	if (0 >= m_UseCount)
		return false;

	if (&Data == m_pRootNode)
	{
		m_pRootNode = Data.pNextNode;
		if (m_pRootNode)
			m_pRootNode->pLastNode = NULL;
	}
	else
	{
		if (Data.pLastNode)
		{
			Data.pLastNode->pNextNode = Data.pNextNode;
		}
		if (Data.pNextNode)
		{
			Data.pNextNode->pLastNode = Data.pLastNode;
		}
	}

	--m_UseCount;
	return true;
}


const RankData* BaseRank::GetDataByIndex(int Index)
{
	if (0 > Index || m_UseCount < Index)
		return NULL;


	return NULL;
}

/*
快速访问
快速查找
*/

class Rank
{
public:
	Rank() {}
	virtual ~Rank() {}

	void Init()
	{
		for (int i = 0; i < 10; ++i)
		{
			m_RankData[i].RankValue = i;
			m_RankData[i].RoleID = i;
			m_IdxRank.insert(std::make_pair(i, &m_RankData[i]));
		}
	}

	int GetRankByID(int Id)
	{
		std::map<int, RankData*>::iterator it = m_IdxRank.find(Id);
		if (it == m_IdxRank.end())
			return -1;

		return it->second - m_RankData;
	}

private:
	RankData m_RankData[10];
	std::map<int, RankData*> m_IdxRank;
};


class A
{
public:
	A() {}
	virtual ~A() {}
};

class C
{
public:
	C() {}
	virtual ~C() {}
};

class B :public A, public C
{
public:
	B() {}
	~B() {}
};


struct TestA
{
	int a;
	char b[3];
};


#include <vector>
#include <iostream>
#include<algorithm>
int main()
{

	//Rank Orank;
	//Orank.Init();

	//for (int i = 0; i < 10; ++i)
	//{
	//	printf("%d\n", Orank.GetRankByID(i));
	//}

	//printf("size A:%d-- size B:%d\n", sizeof(A), sizeof(B));


	UINT32 i = 4294967295;
	printf("%u\n", i);

	int a = 0;
	int b = 1;

	a *= 2 + 3;
	printf("%d\n", a);

	a = a * 2 + 3;
	printf("%d\n", a);


	return 0;
}

#endif