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
#include "../MySvrConnector/stdafx.h"

inline void strncpy_safe(char* des, const int desSize, const char* src, const int srcSize)
{
#ifdef _WIN32
	strncpy_s(des, desSize, src, srcSize);

#else
	strncpy(des, src, size);

#endif // _WIN32
}
