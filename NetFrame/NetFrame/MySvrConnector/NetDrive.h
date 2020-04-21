/************C++ Header File****************
#
#	Filename: NetDrive.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:39:58
#	Last Modified: 2018-08-11 17:39:58	
*******************************************/

#pragma once
#include "Socket.h"
#include "../MyThreadPool/MyMutex.h"


#ifdef _WIN32
typedef unsigned int(__stdcall *ThreadProcess)(void*);
#else
typedef void(*ThreadProcess)(void*);
#endif // _WIN32

namespace NetFrame
{

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

	typedef void (*EventCb)(Socket ev, void* userData);

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



	struct SocketIOEvent
	{
		uint32			fd;
		//char			evBuffer[1024];
		EventHandler	eventHandler;
		void* pUserData;
	};

	class NetDrive
	{
	public:
		virtual ~NetDrive();

	protected:
		NetDrive();

	public:
		//virtual int InitIO(const char* ip, int port, uint32 max_fd) = 0;

		//virtual void WaitEvent() = 0;

		//virtual void HandleEvent(const IOEvent& ioEvent) = 0;

		//virtual void HandleEvent() {}

		virtual int Init() = 0;

		virtual void Launch() = 0;

		static NetDrive* CreateSocketIO(int max_fd, IOType ioType = SI_SELECT);

		static NetDrive* AdapterNetDrive(int maxFd);

	public:
		//const std::map<uint32, Socket>& GetFds() const { return m_sockets; }

		//uint32 GetMaxFd() const { return m_max_socket; }

		//uint32 GetEventSize() { return m_event.size(); }

		//SocketIOEvent* GetEvent(uint32 fd);

		//void AddEvent(uint32 fd, EventCb readCb, EventCb writeCb, EventCb listenCb, EventCb errCb, void* userData);

		//bool EventEmpty() const { return m_event.empty(); }
		//const IOEvent& GetIOEvent() const { return m_event.front(); }
		//void DelIOEvent() { m_event.pop(); }

		//void ProcessEvent();

		//void ProcessListen(Socket& sk);
		//void ProcessRead(Socket& sk);
		//void ProcessWrite(Socket& sk);
		//void ProcessErr(Socket& sk);


	protected:
		//virtual void addIOEvent(const IOEvent& ioEvent) { m_event.push(ioEvent); }

		//virtual void delSocket(const uint32 fd) {}

		//virtual int AddSocket(uint32 fd) { return -1; }

		//virtual int DelSocket(uint32 fd) { return -1; }


	protected:
		//std::map<uint32, Socket>	m_sockets;
		//std::map<uint32, SocketIOEvent> m_events;
		//std::multimap<uint32, IOEvent>	m_ioEvents;
		//uint32 m_max_socket;
		//char m_recv_buf[MAX_RECV_BUF_SIZE];
		//std::queue<IOEvent> m_event;
		//int m_ioType;
	};


	NetDrive* CreateSocketIO(int max_fd, IOType ioType = SI_SELECT);
}
