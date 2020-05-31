#pragma once

#include "../../common/stdafx.h"
#include "Thread.h"
#include "ThreadTask.h"
#include "Mutex.h"
#include "Sem.h"
#include "Condition.h"


class ThreadPool : public NonCopyable
{
public:
	//这个值用来在线程中等待信号的超时时间,不要永久阻塞等待
	//由于在stop的时候发出BroadCast的时候,
	//部分线程可能处m_running判断之后和等待信号之前
	//防止线程池实际已停止,但仍阻塞在等待中
	static const int THREAD_WAIT_TIMEOUT = 10;

	explicit ThreadPool(int nThreadNum = -1);

	~ThreadPool();

	int Run();

	int Stop();

	int PushTask(ThreadTask* pTask);
	
private:
	static THREAD_FUNCTION_PRE PoolWorkFunc(void*);

private:
	std::list<Thread*> m_threads;

	std::queue<ThreadTask*> m_tq;		//任务队列

	uint32 m_threadNum;

	Mutex m_mutex;

	Condition m_cond;

	bool m_running;

};

