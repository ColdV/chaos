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
#include <list>
//#include "Event.h"

#ifdef _WIN32
typedef unsigned int(__stdcall *ThreadProcess)(void*);
#else
typedef void(*ThreadProcess)(void*);
#endif // _WIN32



namespace NetFrame
{

	class EventCentre;

	//enum IOType
	//{
	//	SI_SELECT = 1,
	//	SI_EPOLL = 2,
	//	SI_IOCP = 3
	//};

	//enum
	//{
	//	MAX_RECV_BUF_SIZE = 1024 * 10,
	//};

	//enum SockEvent
	//{
	//	SE_READ = 1,
	//	SE_WRITE = 1 << 1,
	//	SE_EXCEPT = 1 << 2,
	//};

	//typedef void (*EventCb)(Socket ev, void* userData);

	//struct EventHandler
	//{
	//	EventCb		listenCb;
	//	EventCb		readCb;
	//	EventCb		writeCb;
	//	EventCb		errCb;
	//};


	//struct IOEvent
	//{
	//	uint32 fd;
	//	SockEvent sock_event;
	//	EventCb evcb;
	//};



	//struct SocketIOEvent
	//{
	//	uint32			fd;
	//	//char			evBuffer[1024];
	//	EventHandler	eventHandler;
	//	void* pUserData;
	//};


	struct FdEvent
	{
		socket_t fd;
		short ev;
	};

	class NetDrive
	{
	public:
		virtual ~NetDrive();

	//protected:
		NetDrive(/*EventCentre* pCentre = 0*/)//: m_pCentre(pCentre)
		{ 
			m_fds.clear(); 
			m_activeFd.clear(); 
		}

	public:
		virtual int Init() = 0;

		virtual int Launch(/*EventCentre* pCentre*/) = 0;

		void AddFd(socket_t fd, uint32 ev) { m_fds.insert(fd); RegistFd(fd, ev); }

		void DelFd(socket_t fd) { m_fds.erase(fd); CancelFd(fd); }

		const std::list<FdEvent>& GetActives() const { return m_activeFd; }

		void ResetActives() { m_activeFd.clear(); }

		static NetDrive* AdapterNetDrive();


	protected:
		virtual void RegistFd(socket_t fd, short ev) {}

		virtual void CancelFd(socket_t fd) {}

		void PushActiveFd(const FdEvent& fdEv) { m_activeFd.push_back(fdEv); }

		std::set<socket_t>* GetFds() { return &m_fds; }
		

	protected:
		std::set<socket_t> m_fds;
		std::list<FdEvent> m_activeFd;
	};

}
