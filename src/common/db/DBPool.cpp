#include "DBPool.h"

#ifdef DB_MYSQL_ENABLE
#include "dbmysql/DBMysql.h"
#endif // DB_MYSQL_ENABLE

#ifdef DB_REDIS_ENABLE
#include "dbredis/DBRedis.h"
#endif // DB_REDIS_ENABLE


namespace chaos
{
	namespace db
	{
		DBPool::DBPool(const DBConfig& dbConfig, short dbType, int maxConn/* = 4*/) :
			m_maxConn(maxConn),
			m_dbType(dbType),
			m_LiveConnNum(0),
			m_started(false),
			m_threadPool(new ThreadPool(m_maxConn)),
			m_cond(m_mutex)
		{
			memcpy(&m_dbConfig, &dbConfig, sizeof(DBConfig));
		}


		DBPool::~DBPool()
		{
			if (m_started)
				m_threadPool->Stop();
		}


		void DBPool::Start()
		{
			if (m_started)
				return;

			m_threadPool->Run();

			for (int i = 0; i < m_maxConn; ++i)
			{
				m_threadPool->PushTask(std::bind(&DBPool::Work, this));
			}

			m_started = true;
		}


		void DBPool::Stop()
		{
			if (!m_started)
				return;

			m_started = false;

			m_cond.CondBroadCast();
		}


		void DBPool::Query(const std::string& cmd, const DBQueryCb& cb)
		{
			{
				MutexGuard lock(m_mutex);

				m_rq.push({ cmd, cb });
			}

			m_cond.CondBroadCast();
		}


		void DBPool::Recover()
		{
			if (!m_started || m_LiveConnNum >= m_maxConn)
				return;

			int recoverNum = m_maxConn - m_LiveConnNum;
			for (int i = 0; i < recoverNum; ++i)
			{
				m_threadPool->PushTask(std::bind(&DBPool::Work, this));
			}
		}


		void DBPool::Work()
		{
			std::unique_ptr<DBBase> pDBBase(CreateDBBase());
			if (!pDBBase)
				return;

			if (!pDBBase->Connect())
			{
				printf("connect db:%s failed!\n", DBName[m_dbType]);
				return;
			}

			++m_LiveConnNum;

			while (m_started)
			{
				DBRequest req;
				int pingNum = 0;

				{
					MutexGuard lock(m_mutex);

					m_cond.CondWait(DB_PING_TIMEMS);

					if (m_rq.empty())
					{
						if (0 != pDBBase->Ping())
							++pingNum;
						else
							pingNum = 0;

						if (pingNum >= DB_PING_MAX)
						{
							printf("db:%s ping failed!\n", DBName[m_dbType]);
						}
						continue;
					}

					req = m_rq.front();
					m_rq.pop();
				}

				DBResultBase* pResult = CreateDBResultBase();
				if (!pResult)
				{
					printf("error db result!\n");
					return;
				}

				int ret = pDBBase->Query(req.cmd, pResult);

				if (req.requestCb)
					req.requestCb(req.cmd, *pResult, ret);

				delete pResult;
			}

			--m_LiveConnNum;
		}


		DBBase* DBPool::CreateDBBase()
		{
			DBBase* pDBBase = NULL;

			if (DBT_MYSQL == m_dbType)
			{
#ifdef DB_MYSQL_ENABLE
				MysqlConfig& config = m_dbConfig.mysqlConfig;
				pDBBase = new DBMysql(config.dbport, config.dbip, config.dbuser,
					config.dbpwd, config.dbname);
#else
				printf("no found mysql!\n");
#endif // DB_MYSQL_ENABLE
			}
			else if (DBT_REDIS == m_dbType)
			{
#ifdef DB_REDIS_ENABLE
				RedisConfig& config = m_dbConfig.redisConfig;
				pDBBase = new DBRedis(config.dbip, config.dbport);
#else
				printf("no found redis!\n");
#endif // DB_REDIS_ENABLE

			}
			else
				printf("no found db config by db:%s\n", DBName[m_dbType]);

			return pDBBase;
		}


		DBResultBase* DBPool::CreateDBResultBase()
		{
			DBResultBase* pResultBase = NULL;

			if (DBT_MYSQL == m_dbType)
			{
#ifdef DB_MYSQL_ENABLE
				pResultBase = new DBMysqlResult;
#else
				printf("no found mysql!\n");
#endif // DB_MYSQL_ENABLE
			}
			else if (DBT_REDIS == m_dbType)
			{
#ifdef DB_REDIS_ENABLE
				pResultBase = new DBRedisResult;
#else
				printf("no found redis!\n");
#endif // DB_REDIS_ENABLE
			}
			else
				printf("no found db result class by db:%s\n", DBName[m_dbType]);

			return pResultBase;
		}
	}
}	