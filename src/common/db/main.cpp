#include <stdio.h>
#include "../common.h"
#include "dbmysql/DBMysql.h"
#include <time.h>
#include "../package/Package.h"

#define MAX_BUF 102400
#define LOOP 10240


#if 0
int main()
{
	DBMysql m(3306, "127.0.0.1", "root", "root", "game");

	int ret = m.Connect();

	printf("%d\n", ret);

	DBResult res;
	//m.Query("select * from user", sizeof("select * from user"), &res);
	//m.Query("insert into  user_1 values (100, 'abcd');");

	//int rows = 1000000;

	//auto now = time(NULL);
	//for (int i = 1; i <= rows; ++i)
	//{
	//	char sql[1024]{0};
	//	char str[32]{ 0 };
	//	sprintf_s(str, "hello world:%d", i);

	//	snprintf(sql, sizeof(sql), "insert into test value(%d, %d,'%s',%f,%lf,'%s','%s','%s','%s','%s')",
	//		i, i, str, (float)i, (double)i, "0:0:0", str, str, str, str);
	//	int ret = m.Query(sql, strlen(sql), NULL);
	//	if (0 != ret)
	//	{
	//		printf("%s\n", m.GetLastErrStr());
	//		break;
	//	}
	//	printf("result:%d--affect:%d\n", ret, m.GetLastAffectRows());
	//}

	//printf("finish cost:%llu\n", time(NULL) - now);

	//char a[100000000]{ 0 };
	std::vector<char>(100000000);

	auto now = time(NULL);
	char sql[] = "select * from test limit 0, 100";
	ret = m.Query(sql, sizeof(sql), &res);
	printf("ret:%d cost:%d\n", ret, time(NULL) - now);

	return 0;
}

#endif

//#if 0

#include "../template/Single.h"

class test : public Single<test>
{
public:
	test() { printf("test()\n"); }
	~test() { printf("test()\n"); }

private:
	static int a;
};

int test::a = 1;


int main()
{
	test a;

	char str[] = "hello world";
	char pkg[1024]{ 0 };

	uint32 size = Package::Instance().Pack(str, sizeof(str), pkg, sizeof(pkg));

	printf("pack size:%d\n", size);

	char unpkg[128]{ 0 };
	size = Package::Instance().Unpack(pkg, sizeof(pkg), unpkg, sizeof(unpkg));

	printf("unpack size:%d\n", size);

	return 0;
}
//#endif