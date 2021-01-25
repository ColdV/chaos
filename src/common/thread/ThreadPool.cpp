#include <assert.h>
#include "ThreadPool.h"


ThreadPool::ThreadPool(int nThreadNum /*= -1*/) :
	m_threadNum(nThreadNum),
	m_mutex(),
	m_cond(m_mutex),
	m_running(false)
{
	if (0 >= nThreadNum)
	{
#ifdef _WIN32
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		m_threadNum = systemInfo.dwNumberOfProcessors * 2;
#else
		m_threadNum = sysconf(_SC_NPROCESSORS_ONLN);
#endif // _WIN32
	}

	for (uint32 i = 0; i < m_threadNum; ++i)
	{
		m_threads.emplace_back(new Thread(&ThreadPool::PoolWorkFunc, (void*)this));
	}

}


ThreadPool::~ThreadPool()
{
}


int ThreadPool::Run()
{
	if (m_running)
		return -1;
	
	m_running = true;

	for (auto& thread : m_threads)
		thread->Start();

	return 0;
}


int ThreadPool::Stop()
{
	{
		MutexGuard lock(m_mutex);

		if (!m_running)
			return -1;

		m_running = false;

		m_cond.CondBroadCast();
	}

	for (auto& thread : m_threads)
	{
		thread->Join();
	}

	return 0;
}



void ThreadPool::PushTask(const ThreadTask& task)
{
	if (!task || !m_running)
		return;

	MutexGuard lock(m_mutex);

	m_tq.emplace(task);

	m_cond.CondSignal();
}


THREAD_FUNCTION_PRE ThreadPool::PoolWorkFunc(void* arg)
{
	assert(arg);

	ThreadPool* pThreadPool = (ThreadPool*)arg;

	while (pThreadPool->m_running)
	{
		ThreadTask task = NULL;
		{
			MutexGuard lock(pThreadPool->m_mutex);

			while (pThreadPool->m_tq.empty() && pThreadPool->m_running)
			{
				pThreadPool->m_cond.CondWait(/*THREAD_WAIT_TIMEOUT*/);
			}

			//在cond_wait中会因为pool调用stop(置running为false)而跳出循环
			//这里再次判断running状态
			if (!pThreadPool->m_running)
				continue;

			//这里m_tq应该一定不为空
			assert(!pThreadPool->m_tq.empty());
			task = pThreadPool->m_tq.front();

			pThreadPool->m_tq.pop();
		}

		if(task)
			task();

	}

	return 0;
}


