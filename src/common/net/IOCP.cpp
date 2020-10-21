/************C++ Source File****************
#
#	Filename: IOCP.cpp
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:47
#	Last Modified: 2018-08-11 17:40:47
*******************************************/

#ifdef _WIN32

#include "IOCP.h"
#include "Event.h"
#include "log/Logger.h"

namespace chaos
{

	/*IOCP& IOCP::Instance()
	{
		static IOCP s_iocp;
		return s_iocp;
	}*/

	IOCP::AcceptExPtr IOCP::s_acceptEx = NULL;
	//IOCP::ConnectExPtr IOCP::s_connectEx = NULL;
	IOCP::GetAcceptExSockaddrsPtr IOCP::s_getAcceptExSockaddrs = NULL;


	IOCP::IOCP(EventCentre* pCentre):
		Poller(pCentre),
		m_completionPort(0),
		m_isInit(false),
		m_workThreads(0),
		m_threadHandles(0),
		m_liveThreads(0),
		m_tids(0),
		m_pThreadParam(0)
	{
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		m_workThreads = systemInfo.dwNumberOfProcessors * 2;

		m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, m_workThreads);

		m_threadHandles = new HANDLE[m_workThreads]{ 0 };
		if (!m_threadHandles)
			return; 

		m_tids = new thread_t[m_workThreads]{ 0 };
		if (!m_tids)
			return;
	}


	IOCP::~IOCP()
	{
		for (DWORD i = 0; i < m_workThreads; ++i)
		{
			PostQueuedCompletionStatus(m_completionPort, 0, NOTIFY_SHUTDOWN_KEY, NULL);
		}

		if (0 != m_liveThreads)
			m_sem.SemWait();

		if (m_threadHandles)
		{
			for (DWORD i = 0; i < m_workThreads; ++i)
			{
				CloseHandle(m_threadHandles[i]); 
			}
			delete[] m_threadHandles;
		}

		if (m_tids)
			delete[] m_tids;

		if (m_pThreadParam)
			delete m_pThreadParam;

		if (m_completionPort)
			CloseHandle(m_completionPort);
	}


	int IOCP::Init()
	{
		if (m_isInit)
			return 0;

		if (!m_pThreadParam)
		{
			m_pThreadParam = new THREAD_PARAM();
			m_pThreadParam->iocp = m_completionPort;
			m_pThreadParam->pIOCP = this;
		}

		for (DWORD i = 0; i < m_workThreads; ++i)
		{
			m_threadHandles[i] = (HANDLE)_beginthreadex(NULL, 0, &IOCP::Loop, m_pThreadParam, 0, &m_tids[i]);
			if (!m_threadHandles)
				return -1;
		}

		if (!s_acceptEx || /*!s_connectEx ||*/ !s_getAcceptExSockaddrs)
		{
			//获取Ex系列函数
			GUID acceptex = WSAID_ACCEPTEX;
			//GUID connectex = WSAID_CONNECTEX;
			GUID getacceptexsockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
			Socket s(AF_INET, SOCK_STREAM, 0);

			if (INVALID_SOCKET == s.GetFd())
				return -1;

			socket_t fd = s.GetFd();
			DWORD bytes = 0;

			if (0 != WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptex, sizeof(acceptex),
				&s_acceptEx, sizeof(s_acceptEx), &bytes, NULL, NULL))
				return -1;

			/*if (0 != WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &connectex, sizeof(connectex),
				&s_connectEx, sizeof(s_connectEx), &bytes, NULL, NULL))
				return -1;*/

			if (0 != WSAIoctl(fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &getacceptexsockaddrs, sizeof(getacceptexsockaddrs),
				&s_getAcceptExSockaddrs, sizeof(s_getAcceptExSockaddrs), &bytes, NULL, NULL))
				return -1;
		}

		m_isInit = true;

		return 0;
	}


	BOOL IOCP::AcceptEx(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength,
		DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped)
	{
		if (!s_acceptEx)
			return false;
		
		return s_acceptEx(sListenSocket, sAcceptSocket, lpOutputBuffer, dwReceiveDataLength,
			dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, lpOverlapped);
	}


	void IOCP::GetAcceptExSockeaddrs(PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength,
		DWORD dwRemoteAddressLength, LPSOCKADDR* LocalSockaddr, LPINT LocalSockaddrLength, LPSOCKADDR * RemoteSockaddr, LPINT RemoteSockaddrLength)
	{
		if (!s_getAcceptExSockaddrs)
			return;

		s_getAcceptExSockaddrs(lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength,
			LocalSockaddr, LocalSockaddrLength, RemoteSockaddr, RemoteSockaddrLength);
	}


	int IOCP::RegistFd(socket_t fd, short ev)
	{
		HANDLE ret = CreateIoCompletionPort((HANDLE)fd, m_completionPort, NULL/*(DWORD)pKeyData*/, 0);
		if (!ret)
			return -1;

		return 0;
	}


	int IOCP::CancelFd(socket_t fd)
	{
		//if (!CloseHandle((HANDLE)fd))
		//{
		//	LOG_DEBUG("closehandle:%d\n", fd);
		//	return GetLastError();
		//}

		return 0;
	}


	int IOCP::Launch(int timeoutMs)
	{
		if (0 >= timeoutMs)
			timeoutMs = NET_TICK;

		Sleep(timeoutMs);

		return 0;
	}


	unsigned int IOCP::Loop(void* arg)
	{
		_CrtSetBreakAlloc(151);
		_CrtSetBreakAlloc(152);
		if (!arg)
			return -1;

		LPTHREAD_PARAM pThreadParam = (LPTHREAD_PARAM)arg;
		IOCP* iocp = pThreadParam->pIOCP;
		if (!iocp)
			return -1;

		EventCentre& centre = iocp->GetCentre();

		iocp->AddLiveThread();

		while (1)
		{
			DWORD bytes = 0;
			ULONG_PTR key = 0;
			OVERLAPPED *overlapped = NULL;
			bool bOk = GetQueuedCompletionStatus(pThreadParam->iocp, &bytes, &key, &overlapped, WSA_INFINITE);

			//结束GetQueuedCompletionStatus 准备退出工作线程
			if (NOTIFY_SHUTDOWN_KEY == key)
				break;

			if (overlapped)
			{
				LPCOMPLETION_OVERLAPPED lo = (LPCOMPLETION_OVERLAPPED)overlapped;

				if (bOk)
				{
					printf("GetQueuedCompletionStatus sucess!\n");

					//if (0 != bytes)
					//	printf("recv[%d]:%s\n", lo->fd, lo->databufs[0].buf);
				}
				else
				{
					int err = GetLastError();
					if ((err != WAIT_TIMEOUT) && (err != ERROR_NETNAME_DELETED))
						printf("GetQueuedCompletionStatus failed:%d\n", WSAGetLastError());
				}

				if (lo->cb)
				{
					MutexGuard lock(centre.GetMutex());
					lo->cb(overlapped, bytes, key, bOk);
				}
			}

			else
			{
				printf("GetQueuedCompletionStatus return overlapped is null!\n");
				if (bOk)
				{
					printf("GetQueuedCompletionStatus sucess but overlapped is null!\n"); 
				}
				else
				{
					printf("GetQueuedCompletionStatus failed:%d\n", WSAGetLastError());
				}
			}
		}

		if (iocp->DecLiveThread() == 0)
		{
			MutexGuard lock(iocp->m_mutex);
			iocp->m_sem.SemPost();
		}

		return 0;
	}

}

#endif // !_WIN32