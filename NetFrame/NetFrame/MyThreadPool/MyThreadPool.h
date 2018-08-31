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


class MyThreadPool
{
	friend void MyPoolWorkThread::Run();

public:
	MyThreadPool() {}
	MyThreadPool(int nThreadNum);

	~MyThreadPool() {}

	const std::map<unsigned long int, MyPoolWorkThread>& GetAllThreads() const { return m_all_threads; }
	int GetAllThreadCount() const { return m_all_threads.size(); }

	const std::map<unsigned long int, MyPoolWorkThread*>& GetActiveThreads() const { return m_active_threads; }
	int GetActiveThreadsCount() const { return m_active_threads.size(); }

	const std::map<unsigned long int, MyPoolWorkThread*>& GetFreeThreads() const { return m_free_threads; }
	int GetFreeThreadsCount() const { return m_free_threads.size(); }

	const MyPoolWorkThread* FindThread(int nTid) const;

	void AddTask(const MyTask* task); 

	bool IsHaveTask() const { return m_task_queue.empty(); }

	int GetTaskCount() const { return m_task_queue.size(); }

public:
	//static void Run();

private:
	MyPoolWorkThread* FindThread(int nTid);

	bool AddThread(const MyPoolWorkThread&);

	bool AddActiveToFree(const MyPoolWorkThread&);

	bool AddFreeToActive(const MyPoolWorkThread&);

	bool DelThread(int nTid);

	MyTask* GetFrontTask() { return m_task_queue.front(); }

	void DestroyTask();	// { m_task_queue.pop(); }
	

private:
	std::map<unsigned long int, MyPoolWorkThread> m_all_threads;
	std::map<unsigned long int, MyPoolWorkThread* > m_active_threads;
	std::map<unsigned long int, MyPoolWorkThread*> m_free_threads;

	std::queue<MyTask*> m_task_queue;

	MyMutex m_mutex;
#ifdef _WIN32
	MyEvent	m_cond;
#else
	MyCondition m_cond;
#endif // _WIN32

};

