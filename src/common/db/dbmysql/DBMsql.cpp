#include "DBMysql.h"
#include <assert.h>

namespace chaos
{
	namespace db
	{

		DBMysql::DBMysql(uint32 dbport, const char* dbip, const char* dbuser,
			const char* dbpwd, const char* dbname) :
			m_pMysql(NULL),
			m_dbport(dbport),
			m_dbip(dbip),
			m_dbuser(dbuser),
			m_dbpwd(dbpwd),
			m_dbname(dbname)
		{
			m_pMysql = mysql_init(NULL);
		}


		DBMysql::~DBMysql()
		{
			if (m_pMysql)
				mysql_close(m_pMysql);
		}


		bool DBMysql::Connect()
		{
			if (!mysql_real_connect(m_pMysql, m_dbip.c_str(), m_dbuser.c_str(), m_dbpwd.c_str(), 
					m_dbname.c_str(), m_dbport, NULL, 0))
				return false;

			return 0;
		}


		int DBMysql::Query(const std::string& cmd, DBResultBase* result)
		{
			if (!result)
				return -1;

			int ret = mysql_real_query(m_pMysql, cmd.c_str(), (unsigned long)cmd.length());

			if (0 != ret)
				return ret;


			QueryResult(static_cast<DBMysqlResult*>(result));

			return ret;
		}


		int DBMysql::QueryResult(DBMysqlResult* result)
		{
			assert(result);

			MYSQL_RES* pRes = mysql_store_result(m_pMysql);

			if (!pRes)
				return GetLastErrno();

			MYSQL_FIELD* fields = mysql_fetch_fields(pRes);
			if (!fields)
				return GetLastErrno();

			uint32 fieldnum = mysql_num_fields(pRes);

			DBMysqlResult::FieldInfo fieldInfo;
			for (uint32 i = 0; i < fieldnum; ++i)
			{
				fieldInfo.index = i;
				fieldInfo.name = fields[i].name;
				result->BuildName2Field(fields[i].name, fieldInfo);
			}

			MYSQL_ROW row;
			uint64 rownum = 0;

			while (NULL != (row = mysql_fetch_row(pRes)))
			{
				auto lens = mysql_fetch_lengths(pRes);
				for (uint32 i = 0; i < fieldnum; ++i)
				{
					result->AppendField(rownum, i, row[i], lens[i]);
				}

				++rownum;
			}

			mysql_free_result(pRes);

			return 0;
		}

	}	//namespace db
}	//namespace chaos