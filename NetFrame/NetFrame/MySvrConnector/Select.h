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
		enum
		{
			MAX_FD = 1024,
		};

		static Select& Instance(int max_socket);

		virtual ~Select();

		//virtual int InitIO(const char* ip, int port, uint32 max_fd);

		virtual int Init() override;

		virtual void Launch() override;

		virtual void WaitEvent();

		//virtual void HandleEvent(const IOEvent& ioEvent);

	protected:
		Select(int max_socket);

		//virtual int addSocket(const uint32 fd, const Socket& ms, fd_set* rfds, fd_set* wfds, fd_set* efds);
		//void delSocket(const uint32 fd);
		//virtual int AddSocket(uint32 fd);
		//virtual int DelSocket(uint32 fd);
		//virtual void AddFd(uint32 fd, short event) override;

		virtual void RegistFd(socket_t fd, short ev) override;
		
		virtual void CancelFd(socket_t fd, short ev) override;

	private:
		void CollectEvent(const fd_set& rfds, const fd_set& wfds, const fd_set& efds);

	private:
		fd_set m_rfds;
		fd_set m_wfds;
		fd_set m_efds;
	};
}