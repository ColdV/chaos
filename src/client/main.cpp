/************C++ Source File****************
#
#	Filename: test.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-02 16:28:44
#	Last Modified: 2018-08-02 16:28:44
*******************************************/


#include <functional>
#include "common.h"
#include "net/Event.h"
#include "stdafx.h"
#include "log/Logger.h"
#include <memory>

#define MAX_CLIENT	1000
#define IP	"127.0.0.1"
#define PORT	3308

int maxClient = 10000;

static const char MESSAGE[] = "Hello, World!";

static const int SENDMAX = 1024 * 1024 * 1;

static const char BUF[SENDMAX]{ 0 };

class Client
{
public:
	Client();
	~Client() {}

	chaos::Connecter* GetConnecter() { return m_pConnecter; }

	void ConnectCb(chaos::Connecter* pConnecter, int bOk, void* userdata);

	void ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void Send(const char* buf, int buflen);
private:
	chaos::Connecter* m_pConnecter;
	bool m_isEstablished;
	uint64 m_totalReadSize;
};


Client::Client():
	m_pConnecter(new chaos::Connecter(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))),
	m_isEstablished(false),
	m_totalReadSize(0)
{
}


void Client::ConnectCb(chaos::Connecter* pConnecter, int bOk, void* userdata)
{
	static int cnt = 0;
	static int timenow = 0;
	if (0 == cnt)
		timenow = time(NULL);

	++cnt;

	if (!pConnecter)
		return;

	if (!bOk)
	{
		pConnecter->CancelEvent();
		return;
	}

	pConnecter->SetCallback(std::bind(&Client::ReadCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(&Client::WriteCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL, NULL);

	printf("connect sucess::%d, cnt:%d\n", pConnecter->GetSocket().GetFd(), cnt);

	if (maxClient == cnt)
	{
		printf("connect cost time:%d\n", time(NULL) - timenow);
	}

	m_isEstablished = true;
}


void Client::ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	LOG_DEBUG("read socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);

	static int cnt = 0;
	static int timenow = 0;
	static uint64 readSize = 0;
	static uint64 totalReadable = 0;

	if (0 == timenow)
		timenow = time(NULL);

	++cnt;
	readSize += nTransBytes;
	m_totalReadSize += nTransBytes;

	if (0 == nTransBytes)
	{
		ev->CancelEvent();
		return;
	}

	int readable = ev->GetReadableSize() + 1;
	totalReadable += readable - 1;

	printf("total read size:%llu, total readable:%llu\n", m_totalReadSize, totalReadable);

	char* buf = new char[readable];
	if (!buf)
		return;

	memset(buf, 0, readable);

	ev->ReadBuffer(buf, readable);
	//printf("read socket recv data:%s\n", buf);

	if (readSize >= SENDMAX * maxClient)
	{
		printf("read cost time:%d\n", time(NULL) - timenow);
	}
	//ev->Send(MESSAGE, strlen(MESSAGE));
	
	delete[] buf;
}


void Client::WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	LOG_DEBUG("write socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);
}


void Client::Send(const char* buf, int bulen)
{
	static int cnt = 0;
	static int timenow = 0;

	if (0 == cnt)
		timenow = time(NULL);

	++cnt;

	if (!m_isEstablished)
		return;

	int sendSize = m_pConnecter->Send(BUF, sizeof(BUF));

	printf("send size:%d\n", sendSize);

	if (maxClient == cnt)
	{
		printf("send data cost time:%d\n", time(NULL) - timenow);
	}
}


int main(int argc, char** argv)
{
	char ip[16] = { 0 };
	strcpy(ip, IP);
	int port = PORT;

	if (argc > 1 && argv[1])
		strcpy(ip, argv[1]);

	if (argc > 2 && argv[2])
		port = atoi(argv[2]);

	if (argc > 3 && argv[3])
		maxClient = atoi(argv[3]);

	Logger& log = Logger::Instance();
	log.Init("../log", "client", 0);

	chaos::EventCentre* p = new chaos::EventCentre;

	if (0 != p->Init())
	{
		printf("init event centre failed!\n");
		return 0;
	}

	Client* clients = new Client[maxClient];
	if (!clients)
		return 0;

	for (int i = 0; i < maxClient; ++i)
	{
		auto pConnect = clients[i].GetConnecter();
		if (!pConnect)
		{
			printf("err connecter!\n");
			return 0;
		}

		pConnect->SetCallback(NULL,  NULL,
			std::bind(&Client::ConnectCb, &clients[i], std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), NULL);

		p->RegisterEvent(pConnect);

		sockaddr_in sa;
		memset(&sa, 0, sizeof(sa));

		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		inet_pton(AF_INET, ip, (void*)&sa.sin_addr);

		int ret = pConnect->Connect((sockaddr*)&sa, sizeof(sa));
		if (0 != ret)
		{
			printf("socket:%d connect failed. err:%d\n", pConnect->GetSocket().GetFd(), chaos::GetLastErrorno());
			continue;
		}
	}

	for (int i = 0; i < maxClient; ++i)
	{
		clients[i].Send(BUF, sizeof(BUF));
	}

	p->EventLoop();

	delete p;
	delete[] clients;

	return 0;
}
