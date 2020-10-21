#include "Logger.h"
#include <time.h>
#include "common.h"

#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/time.h>
#include <stdarg.h>
#endif // _WIN32


static uint64 GetFileLength(FILE* fp)
{
	if (!fp)
		return 0;

//#ifdef _WIN32
//	return _filelength(_fileno(fp));
//#else
//	struct _stat info;
//	fstat(filepath, &info);
//	return info.st_size;
//#endif //_WIN32
	fseek(fp, 0, SEEK_END);
	return ftell(fp);
}

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
	m_isInit = true;

	return true;
}


void Logger::WriteLog(int logLev, const char* fmt, ...)
{
	if (!m_isInit)
		return;

	if (!fmt || LOG_LEVEL_TRACE > logLev || LOG_LEVEDL_MAX <= logLev)
		return;

	time_t now = time(NULL);

	MutexGuard lock(m_mutex);

	//第一次写文件时打开文件、隔日新建文件
	if (!m_fp || (now / DAY2SEC) != (m_lastSecond / DAY2SEC))
	{
		m_fp = OpenLogFile();
		if (!m_fp)
		{
			return;
		}
	}

	//文件写满,新建一个文件
	if (MAX_SIZE <= m_curSize)
	{
		while (MAX_SIZE <= m_curSize)
		{
			++m_fillFileNum;
			CloseLogFile();
			m_fp = OpenLogFile();	
			if (!m_fp)
			{
				return;
			}
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
	{
		return;
	}

	fflush(m_fp);

	m_lastSecond = now;
	m_curSize += len;
}


FILE* Logger::OpenLogFile()
{
	time_t now = time(NULL);
	tm nowTm;
#ifdef _WIN32
	localtime_s(&nowTm, &now);
#else
	localtime_r(&now, &nowTm);
#endif // _WIN32

	char suffix[64]{ 0 };
	snprintf(suffix, sizeof(suffix), "-%4d%02d%02d.%d.log",
		nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday, m_fillFileNum);

	//std::string fileName = m_fileName + suffix;
	m_curFilePath = m_fileName + suffix;
#ifdef _WIN32
	m_fp = _fsopen(m_curFilePath.c_str(), "ab+", _SH_DENYNO);
#else
	m_fp = fopen(m_curFilePath.c_str(), "ab+");
#endif // _WIN32

	if (!m_fp)
		return NULL;

	m_curSize = GetFileLength(m_fp);

	return m_fp;
}


int Logger::CloseLogFile()
{
	int ret = fclose(m_fp);
	m_fp = NULL;
	return ret;
}


void Logger::FormatTime(time_t t)
{
	int msec = 0;
	tm nowTm;

#ifdef _WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	msec = st.wMilliseconds;
	localtime_s(&nowTm, &t);
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	msec = tv.tv_usec / 1000;
	localtime_r(&t, &nowTm);
#endif // _WIN32

	snprintf(m_fmtTime, sizeof(m_fmtTime), "[%4d-%02d-%02d %02d:%02d:%02d.%03d]",
		nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday,
		nowTm.tm_hour, nowTm.tm_min, nowTm.tm_sec, msec);
}