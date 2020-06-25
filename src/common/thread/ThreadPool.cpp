#include "ThreadPool.h"
#include "ThreadTask.h"
#include <functional>


ThreadPool::ThreadPool(int nThreadNum /*= -1*/) :
	m_threadNum(nThreadNum),
	m_mutex(),
	m_cond(m_mutex),
	m_running(false)
{
	m_threads.clear();

	if (0 > nThreadNum)
	{
#ifdef _WIN32
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		m_threadNum = systemInfo.dwNumberOfProcessors * 2;
#else
		m_threadNum = sysconf(_SC_NPROCESSORS_ONLN);
#endif // _WIN32
	}
}


ThreadPool::~ThreadPool()
{
	for (auto it = m_threads.begin(); it != m_threads.end();)
	{
		delete *it;
		it = m_threads.erase(it);
	}

	while (!m_tq.empty())
	{
		delete m_tq.front();
		m_tq.pop();
	}
}


int ThreadPool::Run()
{
	if (m_running)
		return -1;
	
	m_running = true;

	for (uint32 i = 0; i < m_threadNum; ++i)
	{
		Thread* pThread = new Thread(&ThreadPool::PoolWorkFunc, (void*)this);
		//Thread* pThread = new Thread(std::move(std::bind(&PoolWorkFunc, (void*)this)));
		if (!pThread)
			return -1;

		pThread->Start();

		m_threads.push_back(pThread);
	}

	return 0;
}


int ThreadPool::Stop()
{
	if (!m_running)
		return -1;

	m_running = false;

	m_cond.CondBroadCast();

	for (auto thread : m_threads)
	{
		thread->Join();
	}

	return 0;
}


int ThreadPool::PushTask(ThreadTask* pTask)
{
	if (!pTask)
		return -1;

	m_mutex.Lock();

	m_tq.push(pTask);

	m_cond.CondSignal();

	m_mutex.UnLock();

	return 0;
}


THREAD_FUNCTION_PRE ThreadPool::PoolWorkFunc(void* arg)
{
	if (!arg)
	{
#ifdef _WIN32
		return 1;
#else
		return NULL;
#endif // _WIN32
	}

	ThreadPool* pThreadPool = (ThreadPool*)arg;

	while (pThreadPool->m_running)
	{
		pThreadPool->m_mutex.Lock();

		while (pThreadPool->m_tq.empty() && pThreadPool->m_running)
		{
			pThreadPool->m_cond.CondWait(THREAD_WAIT_TIMEOUT);
		}

		printf("thread[%u] lock!\n", GetCurrentThreadId());

		//在cond_wait中可能因为time_out或者pool调用stop而返回 
		//所以这里还是需要判断下m_tq是否为空
		if (pThreadPool->m_tq.empty())
		{
			pThreadPool->m_mutex.UnLock();
			continue;
		}

		ThreadTask* pTask = pThreadPool->m_tq.front();
		if (!pTask)
		{
			pThreadPool->m_tq.pop();
			pThreadPool->m_mutex.UnLock();
			continue;
		}

		pThreadPool->m_tq.pop();

		pThreadPool->m_mutex.UnLock();

		pTask->Run();

		printf("thread[%u] got!\n", GetCurrentThreadId());

	}

	printf("thread[%u] out!\n", GetCurrentThreadId());

#ifdef _WIN32
	return 0;
#endif // _WIN32
}


