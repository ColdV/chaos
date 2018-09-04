#include "MyThreadPool.h"
#include "MyTask.h"

void MyPoolWorkThread::Run()
{
	while (true)
	{
		if (!m_master)
			continue;

		if (m_master->IsHaveTask())
		{

#ifdef _WIN32
			m_master->m_cond.WaitEvent();
#else
			m_master->CondWait(m_master->m_mutex);
#endif // _WIN32
			m_master->m_mutex.Lock();

			printf("是不是这里!\n");
			MyTask* pTask = m_master->FrontTask();
			m_master->PopTask();

			m_master->m_mutex.UnLock();

			pTask->Run();

			m_master->DestroyTask(pTask);
		}
		
	}
}

/*
void MyCheckTask::Run()
{
	if (!GetParam())
		return;

	MyThreadPool* pThreadPool = (MyThreadPool*)GetParam();

	while (true)
	{
		if (pThreadPool->IsHaveTask())
		{

		}
	}
}
*/

MyThreadPool::MyThreadPool()
{
	Init();
}

MyThreadPool::MyThreadPool(int nThreadNum)
{
	Init();
	for (int i = 0; i < nThreadNum; ++i)
	{
		MyPoolWorkThread* work_thread = new MyPoolWorkThread();
		work_thread->SetMaster(this);
		work_thread->Start();

		AddThread(work_thread);
	}
}


void MyThreadPool::PopTask()
{
	//m_mutex.Lock();

	//delete m_task_queue.front();
	m_task_queue.pop();

	//m_mutex.UnLock();
}

void MyThreadPool::DestroyTask(MyTask* pTask)
{
	delete pTask;
}

void MyThreadPool::AddTask(MyTask* task)
{
	m_task_queue.push(task);

#ifdef _WIN32
	m_cond.SetEvent();
#else
	m_cond.CondBroadCast();
#endif // _WIN32

}

bool MyThreadPool::AddThread(MyPoolWorkThread* work_thread)
{
	if (!work_thread)
		return false;

	std::map<unsigned long int, MyPoolWorkThread*>::iterator it = m_all_threads.insert(std::make_pair(work_thread->GetTid(), work_thread)).first;

	it->second->SetMaster(this);

	m_wait_threads.insert(std::make_pair(work_thread->GetTid(), it->second));

	m_cond.SetEvent();

	return true;
}


bool MyThreadPool::ActiveToWait(const unsigned long int nTid)
{

	auto it = m_all_threads.find(nTid);

	if (it == m_all_threads.end())
		return false;

	m_active_threads.erase(it->first);
	m_wait_threads.insert(std::make_pair(it->first, it->second));

	return true;
}


bool MyThreadPool::WaitToActive(const unsigned long int nTid)
{
	auto it = m_all_threads.find(nTid);

	if (it == m_all_threads.end())
		return false;

	m_wait_threads.erase(it->first);
	m_active_threads.insert(std::make_pair(it->first, it->second));
	
	return true;
}


void MyThreadPool::DelThread(int nTid)
{
	m_all_threads.erase(nTid);
	m_active_threads.erase(nTid);
	m_wait_threads.erase(nTid);
}


MyPoolWorkThread* MyThreadPool::FindThread(int nTid)
{
	auto it = m_all_threads.find(nTid);

	if (it == m_all_threads.end())
		return NULL;
	
	return it->second;
}

const MyPoolWorkThread* MyThreadPool::FindThread(int nTid) const
{
	auto it = m_all_threads.find(nTid);

	if (it == m_all_threads.end())
		return NULL;

	return it->second;
}

void MyThreadPool::Init()
{
	m_all_threads.clear();
	m_active_threads.clear();
	m_wait_threads.clear();
	//m_task_queue.clear();
}