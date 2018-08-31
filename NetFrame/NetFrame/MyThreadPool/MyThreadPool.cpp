#include "MyThreadPool.h"
#include "MyTask.h"

void MyPoolWorkThread::Run()
{
	while (true)
	{
		;
	}
}


MyThreadPool::MyThreadPool(int nThreadNum)
{
	for (int i = 0; i < nThreadNum; ++i)
	{
		MyPoolWorkThread work_thread;
		work_thread.Start();

		std::map<unsigned long int, MyPoolWorkThread>::iterator it = m_all_threads.insert(std::make_pair(work_thread.GetTid(), work_thread)).first;
		m_free_threads.insert(std::make_pair(work_thread.GetTid(), &it->second));
	}
}


void MyThreadPool::DestroyTask()
{
	delete m_task_queue.front();
	m_task_queue.pop();
}