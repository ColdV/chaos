#include "DBMysql.h"

DBMysql::DBMysql(uint32 dbport, const char* dbip, const char* dbuser,
	const char* dbpwd, const char* dbname):
	m_pMysql(0),
	m_dbport(dbport)
{
	strncpy_safe(m_dbip, MAX_DBINFO_BUF, dbip, strlen(dbip));
	strncpy_safe(m_dbuser, MAX_DBINFO_BUF, dbuser, strlen(dbuser));
	strncpy_safe(m_dbpwd, MAX_DBINFO_BUF, dbpwd, strlen(dbpwd));
	strncpy_safe(m_dbname, MAX_DBINFO_BUF, dbname, strlen(dbname));

	m_pMysql = mysql_init(NULL);
}


DBMysql::~DBMysql()
{
	if (m_pMysql)
		mysql_close(m_pMysql);
}


uint32 DBMysql::Connect()
{
	if (!mysql_real_connect(m_pMysql, m_dbip, m_dbuser, m_dbpwd, m_dbname, m_dbport, NULL, 0))
		return GetLastErrno();

	return 0;
}


int DBMysql::Query(const char* cmd, uint32 length, DBResult* pResult)
{
	if (!cmd || !pResult)
		return -1;

	Ping();

	int ret = mysql_real_query(m_pMysql, cmd, length);

	if (0 != ret)
		return ret;

	MYSQL_RES* pRes = mysql_store_result(m_pMysql);

	if (!pRes)
		return GetLastErrno();

	MYSQL_FIELD* fields = mysql_fetch_fields(pRes);
	if(!fields)
		return GetLastErrno();

	uint32 fieldnum = mysql_num_fields(pRes);

	DBResult::FieldInfo fieldInfo;
	for (uint32 i = 0; i < fieldnum; ++i)
	{
		fieldInfo.index = i;
		fieldInfo.name = fields[i].name;
		pResult->BuildName2Field(fields[i].name, fieldInfo);
	}

	MYSQL_ROW row;
	uint64 rownum = 0;

	while (NULL != (row = mysql_fetch_row(pRes)))
	{
		auto lens = mysql_fetch_lengths(pRes);
		for (uint32 i = 0; i < fieldnum; ++i)
		{
			pResult->AppendField(rownum, i, row[i], lens[i]);
		}

		++rownum;
	}

	return 0;
}