#pragma once

#include "../../stdafx.h"
#include "../../common.h"
#include "mysql.h"
#include "DBResult.h"

#ifdef _WIN32
#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "mysqlclient.lib")
#endif // _WIN32


#define MAX_DBINFO_BUF	128

class DBMysql : public NonCopyable
{
public:
	DBMysql(uint32 port, const char* dbip, const char* dbuser, 
		const char* dbpwd, const char* dbname);

	~DBMysql();

	uint32 Connect();

	int Ping() { return mysql_ping(m_pMysql); }

	//执行query语句,并将结果放入DBResult中
	int Query(const char* query, uint32 length, DBResult* pResult);

	//获取上次错误码
	uint32 GetLastErrno() const { return mysql_errno(m_pMysql); }

	//获取上次错误信息
	const char* GetLastErrStr() const { return mysql_error(m_pMysql); }

	//受影响行数
	uint64 GetLastAffectRows() const { return mysql_affected_rows(m_pMysql); }

private:
	//写入执行结果
	int QueryResult(DBResult* pResult);

private:
	MYSQL* m_pMysql;

	uint32 m_dbport;							//数据库端口

	char m_dbip[MAX_DBINFO_BUF];				//数据库ip地址

	char m_dbuser[MAX_DBINFO_BUF];				//数据库用户名

	char m_dbpwd[MAX_DBINFO_BUF];				//数据库密码

	char m_dbname[MAX_DBINFO_BUF];				//数据库名
};