#pragma once

struct Event;

typedef void (*EventCallback)(Event* pEvent, void* userData);


enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
};

struct TimerEvent
{
	unsigned int timerID;
	int timeOut;
	//TimerCallback cb;
};


struct SocketEvent		//need scoketEvMgr
{
	unsigned int fd;
	int size;
	int totalSize;
	char* buffer;
};


struct Event
{
	int evID;
	int ev;

	union
	{
		TimerEvent		evTimer;
		SocketEvent		evSocket;
	} Ev;

	EventCallback	evCb;
	bool isLoop;
	void* userData;
};


class EventNew
{
public:
	virtual ~EventNew() = 0;

protected:
	EventNew() {};

	EventNew* NewEvent(short ev, EventCallback evCb, bool isLoop) {};

protected:
	unsigned int m_id;
	short m_ev;
	EventCallback m_evCb;
	bool m_isLoop;

	static unsigned int m_increment;
	static std::set<unsigned int> m_readyId;
};
