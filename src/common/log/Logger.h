#pragma once

#include "../stdafx.h"
#include "../template/Single.h"

class Logger : public Single<Logger>
{
public:
	static const int MAX_SIZE = 1024 * 1024 * 500;	//日志文件最大大小(500M)

	Logger() {}
	virtual ~Logger() {}

	void WriteLog(const char* log, uint32 size);

private:
	FILE* m_fp;
	int m_size;
};