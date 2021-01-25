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
#define PORT	3307

int maxClient = 1;

static const char MESSAGE[] = "Hello, World!";

static const int SENDMAX = 1024 *1024 * 1;

static const char BUF[SENDMAX]{ 0 };

class Client
{
public:
	Client();
	~Client() {}

	std::shared_ptr<chaos::Connecter>& GetConnecter() { return m_pConnecter; }

	void ConnectCb(chaos::Connecter& pConnecter, int bOk);

	void ReadCb(chaos::Connecter& ev, int nTransBytes);

	void WriteCb(chaos::Connecter& ev, int nTransBytes);

	void Send(const char* buf, int buflen);
private:
	std::shared_ptr<chaos::Connecter> m_pConnecter;
	bool m_isEstablished;
	uint64 m_totalReadSize;
	uint64 m_totalSendSize;
};


Client::Client():
	m_pConnecter(new chaos::Connecter(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))),
	m_isEstablished(false),
	m_totalReadSize(0),
	m_totalSendSize(0)
{
}


void Client::ConnectCb(chaos::Connecter& connecter, int bOk)
{
	static int cnt = 0;
	static int timenow = 0;
	if (0 == cnt)
		timenow = time(NULL);

	++cnt;

	if (!bOk)
	{
		connecter.CancelEvent();
		return;
	}

	connecter.SetReadCallback(std::bind(&Client::ReadCb, this, std::placeholders::_1, std::placeholders::_2));
	connecter.SetWriteCallback(std::bind(&Client::WriteCb, this, std::placeholders::_1, std::placeholders::_2));

	if (maxClient == cnt)
	{
		printf("connect cost time:%d\n", time(NULL) - timenow);
	}

	m_isEstablished = true;

	Send(BUF, sizeof(BUF));
}


void Client::ReadCb(chaos::Connecter& ev, int nTransBytes)
{
	LOG_DEBUG("read socket:%d trans bytes:%d", ev.GetSocket().GetFd(), nTransBytes);

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
		ev.CancelEvent();	
		return;
	}

	int readable = ev.GetReadableSize() + 1;
	totalReadable += nTransBytes;//readable - 1;

	char* buf = new char[readable];
	if (!buf)
		return;

	memset(buf, 0, readable);

	ev.ReadBuffer(buf, readable);
	//printf("read socket recv data:%s\n", buf);
	if (readSize >= SENDMAX * maxClient)
	{
		printf("read cost time:%d\n", time(NULL) - timenow);
	}
	
	delete[] buf;
}


void Client::WriteCb(chaos::Connecter& ev, int nTransBytes)
{
	m_totalSendSize += nTransBytes;
	//LOG_DEBUG("write socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);
	//printf("write socket:%d trans bytes:%d\n", ev->GetSocket().GetFd(), nTransBytes);
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

	//int sendSize = m_pConnecter->Send(BUF, sizeof(BUF));
	int sendSize = m_pConnecter->WriteBuffer(BUF, sizeof(BUF));

	//printf("send size:%d\n", sendSize);

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

		pConnect->SetConnectCallback(std::bind(&Client::ConnectCb, &clients[i], std::placeholders::_1, std::placeholders::_2));

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


	p->EventLoop();

	delete p;
	delete[] clients;

	return 0;
}
