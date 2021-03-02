#pragma once

#include "stdafx.h"
#include "template/Single.h"
#include <functional>
#include <unordered_multimap>


/*********观察者模式实现*********/

namespace chaos
{
	class Objective;
	class PublishPlatform;

	/*观察者*/
	typedef std::function<void(Objective& objective)> Observer;



	/*抽象被观察者*/
	class Objective : public NonCopyable
	{
	public:
		Objective() {}
		virtual ~Objective() = 0;

		//发布事件到platform
		void Publish(PublishPlatform& platform, const std::string& msg);
	};

	
	Objective::~Objective()
	{}


	void Objective::Publish(PublishPlatform& platform, const std::string& msg)
	{
		platform.Publish(msg, *this);
	}



	/*发布平台: 发布和订阅事件*/
	class PublishPlatform : public NonCopyable
	{
	public:
		typedef std::unordered_multimap<std::string msg, Observer observer> ObserverMap;

		PublishPlatform() {}
		~PublishPlatform() {}

		//订阅事件
		void Subscribe(const std::string& msg, const Observer& observer);

		//发布事件
		void Publish(const std::string& msg, Objective& objective);

	private:
		ObserverMap m_observers;
	};


	void PublishPlatform::Subscribe(const std::string& msg, const Observer& observer)
	{
		m_observers.insert(std::make_pair(msg, observer));
	}

	
	void PublishPlatform::Publish(const std::string& msg, Objective& objective)
	{
		auto iters = m_observers.equal_range(msg);
		if(iters.first == m_observers.end())
			return;

		for(auto iter = iters.first; iter != iters.second; ++iter)
		{
			iter->second(objective);
		}
	}

}