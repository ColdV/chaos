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

namespace NetFrame
{

	/*IOCP& IOCP::Instance()
	{
		static IOCP s_iocp;
		return s_iocp;
	}*/

	IOCP::IOCP(EventCentre* pCentre):
		Poller(pCentre),
		m_completionPort(0),
		m_workThreads(0),
		m_threadHandles(0),
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
		for (int i = 0; i < m_workThreads; ++i)
		{
			PostQueuedCompletionStatus(m_completionPort, 0, NOTIFY_SHUTDOWN_KEY, NULL);
		}

		if (m_threadHandles)
		{
			for (int i = 0; i < m_workThreads; ++i)
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

		return 0;
	}


	int IOCP::RegistFd(socket_t fd, short ev)
	{
		CreateIoCompletionPort((HANDLE)fd, m_completionPort, NULL/*(DWORD)pKeyData*/, 0);

		return PushActiveEvent(fd, EV_IOREAD);
	}


	int IOCP::CancelFd(socket_t fd)
	{
		CloseHandle((HANDLE)fd);
		return 0;
	}


	int IOCP::Launch()
	{
		return 0;
	}


	unsigned int IOCP::Loop(void* arg)
	{
		if (!arg)
			return -1;

		LPTHREAD_PARAM pThreadParam = (LPTHREAD_PARAM)arg;

		while (1)
		{
			DWORD bytes = 0;
			ULONG_PTR key = 0;
			OVERLAPPED *overlapped = NULL;
			bool bOk = GetQueuedCompletionStatus(pThreadParam->iocp, &bytes, &key, &overlapped, WSA_INFINITE);

			if (NOTIFY_SHUTDOWN_KEY == key)
				break;

			LPCOMPLETE_OVERLAPPED_DATA data = NULL;
			if (overlapped)
			{
				data = (LPCOMPLETE_OVERLAPPED_DATA)overlapped;
				data->asynRet = bOk;
			}

			if (bOk)
			{
				printf("completion port sucess!\n");

				data->databuf.len = bytes;

				if(0 != bytes)
					printf("recv[%d]:%s\n", data->key.fd, data->databuf.buf);

				if(pThreadParam->pIOCP)
					pThreadParam->pIOCP->PushActiveEvent(data->key.fd, EV_IOREAD);
			}

			else
			{
				printf("iocp failed!\n");
			}

		}

		return 0;
	}

}

#endif // !_WIN32