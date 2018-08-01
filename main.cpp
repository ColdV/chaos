
/*
#include<assert.h>
#include<stdio.h>
#include<memory.h>
#include<stdlib.h>
#include<string>
*/
/*
#include<iostream>
#include<string.h>
#include<vector>
#include<assert.h>
#include<map>
#include<set>
#include <typeinfo>
#include <string>
#include <time.h>
#include <math.h>
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 1
#include<hash_set>
#include <vcruntime_typeinfo.h>
#include<windows.h>
*/
/*
#include <iostream>
#include <string.h>
#include<WinSock2.h>
#include<winsock.h>
#include <memory>
#include <WS2tcpip.h>
#include<map>
using namespace std;
*/
/*
#pragma comment(lib, "libevent.lib")
#pragma comment(lib, "ws2_32.lib")
*/
#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif // !FD_SETSIZE
#include "MySvrConnector.h"

int main()
{ 
	MySvrConnector svr(FD_SETSIZE);
	svr.run();
	return 0;
}