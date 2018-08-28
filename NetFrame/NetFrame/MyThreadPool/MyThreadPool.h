#include "stdafx.h"
#include "MyThread.h"

class MyThreadPool
{
public:
	MyThreadPool() {}
	MyThreadPool(int nThreadNum);
	~MyThreadPool() {}

	bool AddThread(const MyThread&);
	bool AddActiveToFree(const MyThread&);
	bool AddFreeToActive(const MyThread&);
	bool DelThread(int nTid);
	const MyThread& FindThread(int nTid) const;

public:
	static void Run();

private:
	MyThread& FindThread(int nTid);

private:
	std::map<unsigned long int, MyThread> m_all_threads;
	std::map<unsigned long int, MyThread* > m_active_threads;
	std::map<unsigned long int, MyThread*> m_free_threads;
};