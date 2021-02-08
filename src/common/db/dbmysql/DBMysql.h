#pragma once

#include "stdafx.h"
#include "common.h"
#include "mysql.h"
#include "db/DBBase.h"

namespace chaos
{
	namespace db
	{
		class DBMysql : public NonCopyable, public DBBase
		{
		public:
			DBMysql(uint32 port, const char* dbip, const char* dbuser,
				const char* dbpwd, const char* dbname);

			~DBMysql();

			bool Connect() override;

			void DisConnect() override {}

			int Ping() override { return mysql_ping(m_pMysql); }

			//执行query语句,并将结果放入DBMysqlResult中
			int Query(const std::string& cmd, DBResultBase* result) override;

			//获取上次错误码
			uint32 GetLastErrno() const { return mysql_errno(m_pMysql); }

			//获取上次错误信息
			const char* GetLastErrStr() const { return mysql_error(m_pMysql); }

			//受影响行数
			uint64 GetLastAffectRows() const { return mysql_affected_rows(m_pMysql); }

		private:
			//写入执行结果
			int QueryResult(DBMysqlResult* result);

		private:
			MYSQL* m_pMysql;

			uint32 m_dbport;					//数据库端口

			std::string m_dbip;					//数据库ip地址

			std::string m_dbuser;				//数据库用户名

			std::string m_dbpwd;				//数据库密码

			std::string m_dbname;				//数据库名
		};
	}
}