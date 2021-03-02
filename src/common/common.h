/************C++ Header File****************
#
#	Filename: common.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-11 17:07:02
#	Last Modified: 2018-08-11 17:07:02
*******************************************/


#pragma once

#include <string>
#include <string.h>
#include "stdafx.h"

inline void strncpy_safe(char* des, const unsigned int desSize, const char* src, const unsigned int srcSize)
{
	if (!des || !src)
		return;

	if (strlen(src) > desSize && desSize <= srcSize)
		return;

#ifdef _WIN32
	strncpy_s(des, desSize, src, srcSize);

#else
	int size = desSize >= srcSize ? srcSize : desSize;

	strncpy(des, src, size);

#endif // _WIN32
}


inline time_t GetCurrentMSec()
{
#ifdef _WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	return time(NULL) * 1000 + st.wMilliseconds;
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif // _WIN32
}


//tm* GetTM(tm* ptm, const time_t* ptime)
//{
//	if (!ptm || !ptime)
//		return NULL;
//#ifdef _WIN32
//	localtime_s(ptm, ptime);
//#else
//	ptm = localtime(ptime);
//#endif // _WIN32
//	return ptm;
//}


inline int GetCurrentTid()
{
#ifdef _WIN32
	return (int)GetCurrentThreadId();
#else
	return (int)syscall(SYS_gettid);
#endif // _WIN32

}


template<typename T>
inline void SetBit(T& des, int bit) { des |= (1 << bit); }

template<typename T>
inline void ClrBit(T& des, int bit) { des &= (~(1 << bit));}

template<typename T>
inline bool GetBit(T& des, int bit) { return des & (1 << bit); }
