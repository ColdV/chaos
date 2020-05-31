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

inline void strncpy_safe(char* des, const int desSize, const char* src, const int srcSize)
{
	if (!des || !src)
		return;

#ifdef _WIN32
	strncpy_s(des, desSize, src, srcSize);

#else
	int size = desSize >= srcSize ? srcSize : desSize;

	strncpy(des, src, size);

#endif // _WIN32
}


#define SetBit(x, y)	(x |= (1<<y))
#define ClrBit(x, y)	(x&=(~(1<<y)))