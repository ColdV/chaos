#pragma once


#include "db/DBBase.h"
#include "hiredis/hiredis.h"

namespace chaos
{
	namespace db
	{
		class DBRedis :public NonCopyable, public DBBase
		{
		public:
			DBRedis(const std::string& ip, int port);
			~DBRedis();

			bool Connect() override;

			void DisConnect() override;

			int Query(const std::string& cmd, DBResultBase* result) override;

			int Ping() override;

		private:
			//转换请求的结果	
			bool QueryResult(redisReply* reply, DBRedisResult* result);

		private:
			std::string m_ip;

			int m_port;

			redisContext* m_redisContext;

			bool m_connected;
		};


	}	//namespace db
}	//namespace chaos