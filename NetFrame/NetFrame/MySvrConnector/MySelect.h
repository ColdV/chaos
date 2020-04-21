/************C++ Header File****************
#
#	Filename: Select.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:40:22
#	Last Modified: 2018-08-11 17:40:22
*******************************************/

#pragma once

#include "NetDrive.h"
#include "../common/single_templete.h"

namespace NetFrame
{

	class Select : public NetDrive
	{
	public:
		static Select& Instance(int max_socket);

		virtual ~Select();

		virtual int InitIO(const char* ip, int port, uint32 max_fd);

		virtual int Init();

		virtual void Launch();

		//virtual void run();
		virtual void WaitEvent();
		virtual void HandleEvent(const IOEvent& ioEvent);
		virtual void HandleEvent();

	protected:
		Select(int max_socket);

		virtual int addSocket(const uint32 fd, const Socket& ms, fd_set* rfds, fd_set* wfds, fd_set* efds);
		void delSocket(const uint32 fd);
		void CollectEvent(const fd_set& rfds, const fd_set& wfds, const fd_set& efds);

		virtual int AddSocket(uint32 fd);
		virtual int DelSocket(uint32 fd);

	private:
		fd_set m_rfds;
		fd_set m_wfds;
		fd_set m_efds;
	};
}