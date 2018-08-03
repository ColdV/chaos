#include "MySvrConnector.h"

int main()
{ 
	MySvrConnector svr(FD_SETSIZE);
	svr.run();
	return 0;
}