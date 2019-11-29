#include "EventMaster.h"

EventMaster::EventMaster() :m_base(NULL)
//, m_readCb(NULL)
//, m_writeCb(NULL)
//, m_errCb(NULL)
{
}


EventMaster::~EventMaster()
{
}


int EventMaster::Init(const char* ip, int port)
{
	m_base = CreateSocketIO(FD_SETSIZE, SI_SELECT);
	if (!m_base)
		return -1;

	if (0 != m_base->InitIO(ip, port, FD_SETSIZE))
		return -1;

	return 0;
}


int EventMaster::Loop()
{
	//while (true)
	//{
	//	m_base->WaitEvent();

	//	if (0 < m_base->GetEventSize())
	//		printf("wait event!event size:%d\n", m_base->GetEventSize());


	//	while (!m_base->EventEmpty())
	//	{
	//		m_base->HandleEvent(m_base->GetIOEvent());
	//		printf("handle event! event size:%d\n", m_base->GetEventSize());
	//	}
	//}


	while (true)
	{
		m_base->WaitEvent();
		ProcessEvenet();
	}

	return 0;
}


int EventMaster::Stop()
{
	return 0;
}


void EventMaster::RegisterEvent(uint32 fd, EventCb readCb, EventCb writeCb, EventCb listenCb, EventCb errCb, void* userArg)
{
	m_base->AddEvent(fd, readCb, writeCb, listenCb, errCb, userArg);
}


void EventMaster::ProcessEvenet()
{
	while(!m_base->EventEmpty())
		m_base->HandleEvent(m_base->GetIOEvent());
}
