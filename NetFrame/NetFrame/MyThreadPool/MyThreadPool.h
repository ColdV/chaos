#pragma once

#include "stdafx.h"
#include "MyThread.h"
//#include "MyTask.h"
#include "MyMutex.h"

#ifdef _WIN32
#include "MyEvent.h"
#else
#include "MyCondition.h"
#endif // _WIN32


class MyTask;
class MyThreadPool;

class MyPoolWorkThread : public MyThread
{
public:
	MyPoolWorkThread()
	{
		m_master = NULL;
	}

	~MyPoolWorkThread() {}
	
	void SetMaster(MyThreadPool* pMaster) { m_master = pMaster; }

	virtual void Run();

private:
	MyThreadPool* m_master;
};

/*
class MyCheckTask : public MyTask
{
public:
	MyCheckTask() {}
	~MyCheckTask() {}

	virtual void Run() {}
};
*/

class MyThreadPool
{
	//friend void MyPoolWorkThread::Run();
	friend MyPoolWorkThread;

public:
	MyThreadPool();
	MyThreadPool(int nThreadNum);

	~MyThreadPool() {}

	const std::map<unsigned long int, MyPoolWorkThread*>& GetAllThreads() const { return m_all_threads; }
	int GetAllThreadCount() const { return m_all_threads.size(); }

	const std::map<unsigned long int, MyPoolWorkThread*>& GetActiveThreads() const { return m_active_threads; }
	int GetActiveThreadsCount() const { return m_active_threads.size(); }

	const std::map<unsigned long int, MyPoolWorkThread*>& GetWaitThreads() const { return m_wait_threads; }
	int GetWaitThreadsCount() const { return m_wait_threads.size(); }

	const MyPoolWorkThread* FindThread(int nTid) const;

	void AddTask(MyTask* task); 

	bool IsHaveTask() const { return !m_task_queue.empty(); }

	int GetTaskCount() const { return m_task_queue.size(); }

	bool StopAllThread();

public:
	//static void Run();

private:
	void Init();

	MyPoolWorkThread* FindThread(int nTid);

	bool AddThread(MyPoolWorkThread* work_thread);

	bool ActiveToWait(const unsigned long int nTid);

	bool WaitToActive(const unsigned long int nTid);

	void DelThread(int nTid);

	MyTask* FrontTask() { return m_task_queue.front(); }

	void PopTask();	// { m_task_queue.pop(); }

	void DestroyTask(MyTask* pTask);
	

private:
	std::map<unsigned long int, MyPoolWorkThread*> m_all_threads;
	std::map<unsigned long int, MyPoolWorkThread* > m_active_threads;
	std::map<unsigned long int, MyPoolWorkThread*> m_wait_threads;

	std::queue<MyTask*> m_task_queue;

	MyMutex m_mutex;
#ifdef _WIN32
	MyEvent	m_cond;
#else
	MyCondition m_cond;
#endif // _WIN32

};

