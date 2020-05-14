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

namespace NetFrame
{

	class Select : public NetDrive
	{
	public:
		enum
		{
			MAX_FD = 1024,
		};

		//static Select& Instance();
		Select();

		virtual ~Select();

		virtual int Init() override;

		virtual int Launch() override;

	protected:
		virtual void RegistFd(socket_t fd, short ev) override;
		
		virtual void CancelFd(socket_t fd) override;

	private:
		void CollectEvent(const fd_set& rfds, const fd_set& wfds, const fd_set& efds);

	private:
		fd_set m_rfds;
		fd_set m_wfds;
		fd_set m_efds;
	};
}