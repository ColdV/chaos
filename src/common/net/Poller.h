/************C++ Header File****************
#
#	Filename: Poller.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:39:58
#	Last Modified: 2018-08-11 17:39:58	
*******************************************/

#pragma once
#include <unordered_map>
#include "stdafx.h"
#include "event_config.h"

namespace chaos
{

	class Event;
	class EventCentre;

	class Poller : public NonCopyable
	{
	public:
		typedef std::unordered_map<socket_t, std::shared_ptr<Event>> NetEventMap;
		typedef std::vector<Event*> EventList;
		typedef std::shared_ptr<Event> EventSharedPtr;

		Poller(EventCentre* pCentre);

		virtual ~Poller();

	public:
		virtual int Init() = 0;

		virtual int Launch(int timeoutMs, EventList& activeEvents) = 0;

		int AddEvent(const EventSharedPtr& pEvent);

		int DelEvent(Event* pEvent);

		//更新fd监听的事件
		//@ev:需要更新的事件
		void UpdateFd(socket_t fd, short op, short ev);

		Event* GetEvent(socket_t fd);

		EventCentre& GetCentre() const { return *m_pCentre; }

		const NetEventMap& GetAllEvents() const { return m_events; }

		NetEventMap& GetAllEvents() { return m_events; }

		void Clear();

		static Poller* AdapterNetDrive(EventCentre* pCentre);


	protected:
		virtual int RegistFd(socket_t fd, short ev) { return 0; }

		virtual int CancelFd(socket_t fd, short ev) { return 0; }

		int PushActiveEvent(socket_t fd, short ev, EventList& activeEvents);

	private:
		Poller(const Poller&);

	protected:
		EventCentre* m_pCentre;
		NetEventMap m_events;
	};

}
