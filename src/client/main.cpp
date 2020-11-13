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

#define MAX_CLIENT	10000
#define IP	"192.168.0.101"
#define PORT	3307


class Client
{
public:
	Client();
	~Client() {}

	chaos::Connecter* GetConnecter() { return m_pConnecter; }

	void ConnectCb(chaos::Connecter* pConnecter, int bOk, void* userdata);

	void ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

	void WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata);

private:
	chaos::Connecter* m_pConnecter;
};


Client::Client():
	m_pConnecter(new chaos::Connecter(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
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

	if (MAX_CLIENT == cnt)
	{
		printf("accept cost time:%d\n", time(NULL) - timenow);
	}
}


void Client::ReadCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	LOG_DEBUG("read socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);

	if (0 == nTransBytes)
	{
		ev->CancelEvent();
		return;
	}

	int readable = ev->GetReadableSize() + 1;
	char* buf = new char[readable];
	if (!buf)
		return;

	memset(buf, 0, readable);

	ev->ReadBuffer(buf, readable);
	int sendSize = ev->Send(buf, readable);

	LOG_DEBUG("read socket recv data:%s, send size:%d\n", buf, sendSize);
	delete[] buf;
}


void Client::WriteCb(chaos::Connecter* ev, int nTransBytes, void* userdata)
{
	LOG_DEBUG("write socket:%d trans bytes:%d", ev->GetSocket().GetFd(), nTransBytes);
}



int main()
{
	Logger& log = Logger::Instance();
	log.Init("../log", "client", 0);

	chaos::EventCentre* p = new chaos::EventCentre;

	if (0 != p->Init())
	{
		printf("init event centre failed!\n");
		return 0;
	}

	Client* clients = new Client[MAX_CLIENT];
	if (!clients)
		return 0;

	for (int i = 0; i < MAX_CLIENT; ++i)
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
		sa.sin_port = htons(PORT);
		inet_pton(AF_INET, IP, (void*)&sa.sin_addr);

		int ret = pConnect->Connect((sockaddr*)&sa, sizeof(sa));
		if (0 != ret)
		{
			printf("socket:%d connect failed. err:%d\n", pConnect->GetSocket().GetFd(), chaos::GetLastErrorno());
			continue;
		}
	}

	p->EventLoop();

	return 0;
}
