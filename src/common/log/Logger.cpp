#include "Logger.h"
#include <io.h>
#include <iostream>
#include <string>
#include <time.h>
#include "../common.h"

Logger::Logger():
m_fp(NULL),
m_curSize(0),
m_level(0),
m_fillFileNum(0),
m_fmtTime{0},
m_lastSecond(0)
{
}


Logger::~Logger()
{
	if (m_fp)
		fclose(m_fp);
}


bool Logger::Init(const std::string& name, int logLevel)
{
	m_fileName = name;
	m_level = logLevel;

	return true;
}


void Logger::WriteLog(int logLev, const char* fmt, ...)
{
	if (!fmt || LOG_LEVEL_TRACE > logLev || LOG_LEVEDL_MAX <= logLev)
		return;

	time_t now = time(NULL);

	//第一次写文件时打开文件、隔日新建文件
	if (!m_fp || (now / DAY2SEC) != (m_lastSecond / DAY2SEC))
	{
		tm nowTm;
		localtime_s(&nowTm, &now);
		char timeStr[32]{ 0 };
		snprintf(timeStr, sizeof(timeStr), "-%4d%02d%02d",
			nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday);

		m_fp = OpenFile(m_fileName + timeStr + "." + std::to_string(++m_fillFileNum) + ".log");
		if (!m_fp)
			return;

		m_curSize = _filelength(_fileno(m_fp));
	}

	//文件写满,新建一个文件
	if (MAX_SIZE <= m_curSize)
	{
		while (MAX_SIZE <= m_curSize)
		{
			CloseFile();
			m_fp = OpenFile((m_fileName + "." + std::to_string(++m_fillFileNum) + ".log").c_str());
			
			if (!m_fp)
				return;

			m_curSize = _filelength(_fileno(m_fp));
		}
	}

	FormatTime(now);

	std::string msg;
	msg.reserve(256);

	msg.append(m_fmtTime).\
		append("[").\
		append(s_logLevel2String[logLev]).\
		append("]").\
		append(fmt);

	va_list ap;
	va_start(ap, fmt);
	int len = vfprintf(m_fp, msg.c_str(), ap);
	va_end(ap);

	if (len <= 0)
		return;

	fflush(m_fp);

	m_lastSecond = now;
	m_curSize += len;
}


FILE* Logger::OpenFile(const std::string& name)
{
#ifdef _WIN32
	return _fsopen(name.c_str(), "ab+", _SH_DENYNO);
#else
	return fopen(name.c_str(), "ab+");
#endif // _WIN32

}


int Logger::CloseFile()
{
	int ret = fclose(m_fp);
	m_fp = NULL;
	return ret;
}


void Logger::FormatTime(time_t t)
{
	int msec = 0;
#ifdef _WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	msec = st.wMilliseconds;
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	msec = tv.tv_usec / 1000;
#endif // _WIN32

	tm nowTm;
	localtime_s(&nowTm, &t);

	snprintf(m_fmtTime, sizeof(m_fmtTime), "[%4d-%02d-%02d %02d:%02d:%02d.%03d]",
		nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday,
		nowTm.tm_hour, nowTm.tm_min, nowTm.tm_sec, msec);
}