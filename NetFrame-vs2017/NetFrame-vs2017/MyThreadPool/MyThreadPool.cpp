#include "MyThreadPool.h"
#include "MyTask.h"


void MyPoolWorkThread::Run()
{
	printf("进入Run!\n");

	while (true)
	{
		//printf("这里一直在执行！\n");
		/*
		if (!m_master)
			continue;
		*/
		if (m_master->IsHaveTask())
		{
			m_master->m_mutex.Lock();

#ifdef _WIN32
			m_master->m_cond.WaitEvent();
#else
			m_master->m_cond.CondWait(m_master->m_mutex);
#endif // _WIN32
			
			printf("是不是这里!\n");
			MyTask* pTask = m_master->FrontTask();
			m_master->PopTask();

			m_master->m_mutex.UnLock();

			pTask->Run();

			m_master->DestroyTask(pTask);
		}
		
		//printf("i am thread:%d\n", GetCurrentThreadId());
		//Sleep(100000);
	}

	printf("这里退出了！\n");
	
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


MyThreadPool::~MyThreadPool()
{
	
	ClearUp();
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

#ifdef _WIN32
	m_cond.SetEvent();
#endif
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

bool MyThreadPool::StopAllThread()
{
	return true;
}

void MyThreadPool::ClearUp()
{
	for (auto it = m_all_threads.begin(); it != m_all_threads.end(); ++it)
	{
		//it->second->Stop();
		delete it->second;
		
	}
}
