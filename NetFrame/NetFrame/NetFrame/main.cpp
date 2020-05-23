
#ifdef _WIN32
#include <process.h>
#endif // _WIN32

#include "MinHeap.h"
#include <map>
#include "Event.h"
#include "Timer.h"
#include "../../common/common.h"
#include "IOCP.h"
#include <Mswsock.h>
//#include "NetDrive.h"

#pragma comment(lib,"Mswsock.lib")
const char IP[] = "10.246.60.164";//"0.0.0.0";//"10.246.60.179";




int main()
{
	printf("Listener:%d, Connecter:%d socket:%d, center:%d\n", sizeof(NetFrame::Listener), sizeof(NetFrame::Connecter),\
		sizeof(NetFrame::Socket), sizeof(NetFrame::EventCentre));

	NetFrame::EventCentre* pCentre = new NetFrame::EventCentre();

	if (!pCentre)
		return -1;

	if (0 != pCentre->Init())
		return -1;

	struct timeval timeout { 100, 0 };//{0, NetFrame::NET_TICK * 1000};

	NetFrame::Socket* s = new NetFrame::Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP, true);

	if (!s)
		return -1;

	s->Bind("192.168.0.101", 3307);
	s->Listen();

	NetFrame::EventKey* pNKey = new NetFrame::EventKey();
	pNKey->fd = s->GetFd();

	NetFrame::Listener* netEv = new NetFrame::Listener(pCentre, s, EV_IOREAD| EV_IOEXCEPT, pNKey);
	if (!netEv)
		return -1;



	pCentre->RegisterEvent(netEv);

	NetFrame::EventKey* pKey = new NetFrame::EventKey();
	pKey->timerId = NetFrame::Timer::CreateTimerID();
	NetFrame::TimerEvent* ev = new NetFrame::TimerEvent(pCentre, EV_TIMEOUT, pKey, 1, true);

	//pCentre->RegisterEvent(ev);
	pCentre->EventLoop();

	return 0;
}

