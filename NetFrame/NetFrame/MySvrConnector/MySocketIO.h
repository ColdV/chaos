/************C++ Header File****************
#
#	Filename: MySocketIO.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:39:58
#	Last Modified: 2018-08-11 17:39:58	
*******************************************/

#pragma once
#include "MySocket.h"
#include "../MyThreadPool/MyMutex.h"

#ifdef _WIN32
typedef unsigned int(__stdcall *ThreadProcess)(void*);
#else
typedef void(*ThreadProcess)(void*);
#endif // _WIN32


enum IOType
{
	SI_SELECT = 1,
	SI_EPOLL = 2,
	SI_IOCP = 3
};

enum
{
	MAX_RECV_BUF_SIZE = 1024 * 10,
};

enum SockEvent
{
	SE_READ = 1,
	SE_WRITE = 1 << 1,
	SE_EXCEPT = 1 << 2,
};

typedef void (*EventCb)(MySocket ev, void* userData);

struct EventHandler
{
	EventCb		listenCb;
	EventCb		readCb;
	EventCb		writeCb;
	EventCb		errCb;
};


struct IOEvent
{
	uint32 fd;
	SockEvent sock_event;
	EventCb evcb;
};


//struct Event
//{
//	uint32 fd;
//	int	ev;
//	EventCb cb;
//	void* pUserData;
//};

//fd -> event

struct SocketIOEvent
{
	uint32			fd;
	//char			evBuffer[1024];
	EventHandler	eventHandler;	
	void*			pUserData;
};

class MySocketIO
{
public:
	MySocketIO();
	virtual ~MySocketIO();

public:
	virtual int InitIO(const char* ip, int port, uint32 max_fd) = 0;

	virtual void WaitEvent() = 0;

	virtual void HandleEvent(const IOEvent& ioEvent) = 0;

	virtual void HandleEvent() {}

public:
	const std::map<uint32, MySocket>& GetFds() const { return m_sockets; }

	uint32 GetMaxFd() const { return m_max_socket; }

	uint32 GetEventSize() { return m_event.size(); }

	SocketIOEvent* GetEvent(uint32 fd);

	void AddEvent(uint32 fd, EventCb readCb, EventCb writeCb, EventCb listenCb, EventCb errCb, void* userData);

	/*{ 
		//LockReadQueue();  
		int event_size = m_event.size() + m_r_event.size() + m_w_event.size(); 
		//UnLockReadQueue(); 

		return event_size;
	}
	*/

	bool EventEmpty() const { return m_event.empty(); }
	const IOEvent& GetIOEvent() const { return m_event.front(); }
	void DelIOEvent() { m_event.pop(); }

	void ProcessEvent();

	void ProcessListen(MySocket& sk);
	void ProcessRead(MySocket& sk);
	void ProcessWrite(MySocket& sk);
	void ProcessErr(MySocket& sk);

	/*
	void LockReadQueue() { m_r_mutex.Lock(); }
	void UnLockReadQueue() { m_r_mutex.UnLock(); }
	bool ReadEventEmpty() const { return m_r_event.empty(); }
	const IOEvent& GetReadEvent() const{ return m_r_event.front(); }
	void PopReadEvent() { printf("„h³ýÊÂ¼þ!\n"); m_r_event.pop(); }
	

	void LockWriteQueue() { m_w_mutex.Lock(); }
	void UnLockWriteQueue() { m_w_mutex.UnLock(); }
	*/

protected:
	virtual void addIOEvent(const IOEvent& ioEvent) { m_event.push(ioEvent); }

	virtual void delSocket(const uint32 fd) {}

	virtual int AddSocket(uint32 fd) { return -1; }

	virtual int DelSocket(uint32 fd) { return -1; }

	//virtual bool InitIOThread();

/*
#ifdef _WIN32
	static unsigned int __stdcall ReadThreadProcess(void*);

	static unsigned int __stdcall WriteThreadProcess(void*);
#else
	static void ReadThreadProcess(void*);

	static void WriteThreadProcess(void*);
#endif // _WIN32
*/

protected:
	std::map<uint32, MySocket>	m_sockets;
	std::map<uint32, SocketIOEvent> m_events;
	std::multimap<uint32, IOEvent>	m_ioEvents;
	uint32 m_max_socket;
	char m_recv_buf[MAX_RECV_BUF_SIZE];
	std::queue<IOEvent> m_event;
	int m_ioType;

	/*
	std::queue<IOEvent> m_r_event;
	MyMutex	m_r_mutex;
	unsigned long int m_r_tid;

	std::queue<IOEvent>	m_w_event;
	MyMutex m_w_mutex;
	unsigned long int m_w_tid;
	*/
};


MySocketIO* CreateSocketIO(int max_fd, IOType ioType  = SI_SELECT);
