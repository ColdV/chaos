/************C++ Header File****************
#
#	Filename: IOCP.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:43
#	Last Modified: 2018-08-11 17:40:43
*******************************************/
#pragma once


#ifdef _WIN32

#define	IOCP_ENABLE

#include "Poller.h"
#include <functional>
#include "../thread/Sem.h"
#include "../thread/Mutex.h"


namespace chaos
{

	const int NOTIFY_SHUTDOWN_KEY = -1;

	typedef std::function<void(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok)> IOCP_CALLBACK;

#define MAX_WSABUFS 8

	typedef struct
	{
		OVERLAPPED overlapped;
		WSABUF databufs[MAX_WSABUFS];
		socket_t fd;
		IOCP_CALLBACK	cb;
		uint16 eventDestroy : 1;
	}COMPLETION_OVERLAPPED, *LPCOMPLETION_OVERLAPPED;


	typedef struct
	{
		COMPLETION_OVERLAPPED overlapped;
		socket_t acceptfd;
		short inListenerPos;			//在listener的ACCEPT_OVERLAPPED数组中的位置
		int* refcnt;					//在listener中投递AcceptEx引用的该结构的个数(listener中的每个ACCEPT_OVERLAPPED此字段都指向同一个数值)
	}ACCEPT_OVERLAPPED, *LPACCEPT_OVERLAPPED;

	class IOCP;

	typedef struct
	{
		HANDLE iocp;
		IOCP* pIOCP;
	}THREAD_PARAM, *LPTHREAD_PARAM;


	class IOCP : public Poller
	{
	public:
		typedef BOOL(WINAPI *AcceptExPtr)(SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
		typedef BOOL(WINAPI *ConnectExPtr)(SOCKET, const struct sockaddr *, int, PVOID, DWORD, LPDWORD, LPOVERLAPPED);
		typedef void (WINAPI *GetAcceptExSockaddrsPtr)(PVOID, DWORD, DWORD, DWORD, LPSOCKADDR *, LPINT, LPSOCKADDR *, LPINT);

		//static IOCP& Instance();
		IOCP(EventCentre* pCentre);
		~IOCP();

		virtual int Init();

		virtual int Launch(int timeoutMs) override;

		static BOOL AcceptEx(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, 
			DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped);

		//static BOOL ConnectEx(const LPCOMPLETION_OVERLAPPED pOverlapped);

		static void GetAcceptExSockeaddrs(PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, 
			DWORD dwRemoteAddressLength, LPSOCKADDR* LocalSockaddr, LPINT LocalSockaddrLength, LPSOCKADDR * RemoteSockaddr, LPINT RemoteSockaddrLength);

	protected:
		virtual int RegistFd(socket_t fd, short ev) override;

		virtual int CancelFd(socket_t fd) override;

	public:
		static unsigned int __stdcall Loop(void* arg);

	private:
		int AddLiveThread() { MutexGuard lock(m_mutex); return ++m_liveThreads; }

		int DecLiveThread() { MutexGuard lock(m_mutex); return --m_liveThreads; }

	private:
		HANDLE m_completionPort;

		bool m_isInit;

		DWORD m_workThreads;				//线程数量

		HANDLE* m_threadHandles;			//所有线程

		int m_liveThreads;				//活动线程数量

		thread_t* m_tids;					//所有线程ID

		LPTHREAD_PARAM m_pThreadParam;		//线程参数

		Sem m_sem;

		Mutex m_mutex;

		static AcceptExPtr	s_acceptEx;

		//static ConnectExPtr s_connectEx;

		static GetAcceptExSockaddrsPtr s_getAcceptExSockaddrs;
	};

}


#endif // _WIN32
