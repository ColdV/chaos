#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>

#else
#include <pthread.h>

#endif // _WIN32
