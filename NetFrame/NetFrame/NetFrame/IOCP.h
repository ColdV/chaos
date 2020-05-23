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

#include "NetDrive.h"


namespace NetFrame
{
	enum
	{
		INVALID_IOCP_RET = -1,
	};

	typedef struct
	{
		socket_t fd;
	}COMPLETE_KEY_DATA, *LPCOMPLETE_KEY_DATA;


	typedef struct
	{
		OVERLAPPED overlapped;
		WSABUF databuf;
		COMPLETE_KEY_DATA key;
		int asynRet;
	}COMPLETE_OVERLAPPED_DATA, *LPCOMPLETE_OVERLAPPED_DATA;


	class IOCP;

	typedef struct
	{
		HANDLE iocp;
		IOCP* pIOCP;
	}THREAD_PARAM, *LPTHREAD_PARAM;


	class IOCP : public NetDrive
	{
	public:
		//static IOCP& Instance();
		IOCP(EventCentre* pCentre);
		~IOCP();

		virtual int Init();

		virtual int Launch();

	protected:
		virtual int RegistFd(socket_t fd, short ev) override;

		virtual int CancelFd(socket_t fd) override;

	public:
		static void Loop(void* param);

	private:
		HANDLE m_completionPort;
		DWORD m_workThreads;
		HANDLE* m_threadHandles;
		LPTHREAD_PARAM m_pThreadParam;
	};

}


#endif // _WIN32
