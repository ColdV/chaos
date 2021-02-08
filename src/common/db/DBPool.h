#pragma once

#include "stdafx.h"
#include "DBBase.h"
#include "thread/ThreadPool.h"

namespace chaos
{
	namespace db
	{
		//数据库类型
		enum DBType
		{
			DBT_MYSQL = 1,
			DBT_REDIS,
		};	

		static const char* DBName[] = { "mysql", "redis" };


		//数据库配置
		struct MysqlConfig														
		{
			char dbip[MAX_DB_IP];
			uint32 dbport;				
			char dbuser[MAX_DB_USER];				//用户名
			char dbpwd[MAX_DB_PWD];					//密码
			char dbname[MAX_DB_NAME];				//数据库名
			char dbcharacter[MAX_DB_CHARACTER];		//字符集
		};

		struct RedisConfig
		{
			char dbip[MAX_DB_IP];
			uint32 dbport;
		};

		union DBConfig
		{
			MysqlConfig mysqlConfig;
			RedisConfig redisConfig;
		};


		typedef std::function<void(const std::string & cmd, DBResultBase& result, int errorno)> DBQueryCb;

		struct DBRequest
		{
			std::string cmd;
			DBQueryCb requestCb;
		};


		class DBPool : public NonCopyable
		{
		public:
			const int DB_PING_TIMEMS = 10000;		//每10s PING一次数据库
			const int DB_PING_MAX = 10;				//PING最大次数,超过该次数就放弃该连接

			DBPool(const DBConfig& dbConfig, short dbType, int maxConn = 4);
			~DBPool();

			void Start();

			void Stop();

			void Query(const std::string& cmd, const DBQueryCb& cb);

			int GetLiveConnNum() const { return m_LiveConnNum; }

			//将池中的连接数恢复到maxConn
			void Recover();

		private:
			void Work();

			DBBase* CreateDBBase();

			DBResultBase* CreateDBResultBase();

		private:
			//std::vector<DBBase*> m_pool;								//所有数据库连接

			const int m_maxConn;										//连接数

			const short m_dbType;										//池中的数据库类型

			DBConfig m_dbConfig;										//数据库配置

			std::atomic<int> m_LiveConnNum;								//存活的连接数量

			std::atomic<bool> m_started;

			std::unique_ptr<ThreadPool> m_threadPool;					//工作线程(每一个线程维护一个连接)

			std::queue<DBRequest> m_rq;									//请求队列

			Mutex m_mutex;

			Condition m_cond;
		};
	}
}