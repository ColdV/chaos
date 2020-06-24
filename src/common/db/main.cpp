#include <stdio.h>
#include "../common.h"
#include "dbmysql/DBMysql.h"
#include <time.h>
#include "../package/Package.h"
#include <fstream>
#include <io.h>

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

#include "../log/Logger.h"

int main()
{
	time_t t = time(NULL);
	tm local;
	localtime_s(&local, &t);
	char buf[128]{ 0 };

	strftime(buf, sizeof(buf), "%Y-%m-%d", &local);

	printf("%s\n", buf);
	
	//std::string a(__FUNCTION__);
	//std::string b(std::to_string(__LINE__));
	//std::string c(__FILE__);
	//c.reserve(256);
	//std::string e = a + b + c;
	//e.append(__FUNCTION__).append(std::to_string(__LINE__)).append(__FILE__);

	Logger& log = Logger::Instance();
	log.Init("./log", 0);
	
	int loop = 10000;

	while (loop > 0)
	{
		LOG_TRACE("hello world!:%d", loop);
		--loop;
	}

	printf("log end!\n");

	while (true)
	{
		Sleep(3000);
	}

	return 0;
}
//#endif


#if 0
int main()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	printf("%d:%d\n", st.wSecond, st.wMilliseconds);

	tm nowTM;
	time_t now = time(NULL);
	localtime_s(&nowTM, &now);

	printf("%d\n", nowTM.tm_mday);
	return 0;
}
#endif