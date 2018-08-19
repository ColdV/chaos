#include "MySvrConnector.h"
#include "MySelect.h"

int main()
{ 
	MySvrConnector svr(FD_SETSIZE);
	svr.run();
	return 0;
}