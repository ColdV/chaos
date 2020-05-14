
#ifdef _WIN32
#include <process.h>
#endif // _WIN32

#include "MinHeap.h"
#include <map>
#include "Event.h"
#include "Timer.h"


const char IP[] = "10.246.60.164";//"0.0.0.0";//"10.246.60.179";


class TestNonCpy
{
public:
	TestNonCpy(const TestNonCpy&) = delete;
	void operator=(TestNonCpy&) = delete;
	TestNonCpy(int value) :m_value(value)
	{}
	~TestNonCpy() {}

private:
	int m_value;
};

void testNonCpy(TestNonCpy cpy)
{
	;
}
#include "../common/common.h"


int main()
{
	NetFrame::EventCentre* pCentre = new NetFrame::EventCentre();
	if (!pCentre)
		return -1;

	if (0 != pCentre->Init())
		return -1;
	
	socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	NetFrame::Socket* s = new NetFrame::Socket(sock);
	if (!s)
		return -1;

	s->Bind("0.0.0.0", 3307);
	s->Listen();

	NetFrame::EventKey* pNKey = new NetFrame::EventKey();
	pNKey->fd = sock;

	NetFrame::NetEvent* netEv = new NetFrame::NetEvent(pCentre, s, EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT, pNKey);
	if (!netEv)
		return -1;

	pCentre->RegisterEvent(netEv);

	NetFrame::EventKey* pKey = new NetFrame::EventKey();
	pKey->timerId = NetFrame::Timer::CreateTimerID();
	NetFrame::TimerEvent* ev = new NetFrame::TimerEvent(pCentre, EV_TIMEOUT, pKey, 1, true);

	pCentre->RegisterEvent(ev);
	pCentre->EventLoop();

	return 0;
}

