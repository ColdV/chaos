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

#include "Poller.h"


namespace chaos
{
	enum
	{
		INVALID_IOCP_RET = -1,
	};

	const int NOTIFY_SHUTDOWN_KEY = -1;

	//typedef struct
	//{
	//	socket_t fd;
	//}COMPLETE_KEY_DATA, *LPCOMPLETE_KEY_DATA;


	typedef struct
	{
		OVERLAPPED overlapped;
		WSABUF databuf;
		//COMPLETE_KEY_DATA key;
		socket_t fd;
		int32 bytes;		//存储GetQueuedCompletionStatus返回的bytes
		int asynRet;
	}COMPLETE_OVERLAPPED_DATA, *LPCOMPLETE_OVERLAPPED_DATA;


	typedef struct
	{
		COMPLETE_OVERLAPPED_DATA overlapped;
		socket_t acceptfd;
	}ACCEPT_OVERLAPPED_DATA, *LPACCEPT_OVERLAPPED_DATA;

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

		virtual int Launch();

		static BOOL AcceptEx(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, 
			DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped);

		//static BOOL ConnectEx(const LPCOMPLETE_OVERLAPPED_DATA pOverlapped);

		static void GetAcceptExSockeaddrs(PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, 
			DWORD dwRemoteAddressLength, LPSOCKADDR* LocalSockaddr, LPINT LocalSockaddrLength, LPSOCKADDR * RemoteSockaddr, LPINT RemoteSockaddrLength);

	protected:
		virtual int RegistFd(socket_t fd, short ev) override;

		virtual int CancelFd(socket_t fd) override;

	public:
		static unsigned int __stdcall Loop(void* arg);

	private:
		HANDLE m_completionPort;

		DWORD m_workThreads;				//线程数量

		HANDLE* m_threadHandles;			//所有线程

		thread_t* m_tids;					//所有线程ID

		LPTHREAD_PARAM m_pThreadParam;		//线程参数

		static AcceptExPtr	s_acceptEx;

		//static ConnectExPtr s_connectEx;

		static GetAcceptExSockaddrsPtr s_getAcceptExSockaddrs;
	};

}


#endif // _WIN32
