#include <stdio.h>
#include "../common.h"
#include "dbmysql/DBMysql.h"

#define MAX_BUF 102400
#define LOOP 10240


//#if 0
int main()
{
	DBMysql m(3306, "127.0.0.1", "root", "root", "game");

	int ret = m.Connect();

	printf("%d\n", ret);

	DBResult res;
	m.Query("select * from user", sizeof("select * from user"), &res);
	//m.Query("insert into  user_1 values (100, 'abcd');");
	return 0;
}

//#endif

#if 0

class test : public NonCopyable
{
public:
	test() { printf("test()\n"); }
	~test() { printf("test()\n"); }
};

int main()
{
	std::vector<int> a(10);

	//a.insert(std::make_pair(1, t));

	printf("hello world!:%d\n", a[9]);

	return 0;
}
#endif