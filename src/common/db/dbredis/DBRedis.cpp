#include "DBRedis.h"
#include <assert.h>

namespace chaos
{
	namespace db
	{
		DBRedis::DBRedis(const std::string& ip, int port) :
			m_ip(ip),
			m_port(port),
			m_redisContext(NULL),
			m_connected(false)
		{
		}


		DBRedis::~DBRedis()
		{
			redisFree(m_redisContext);
		}


		bool DBRedis::Connect()
		{
			if (m_redisContext)
				return true;

			m_redisContext = redisConnect(m_ip.c_str(), m_port);

			if (!m_redisContext) 
				return false;

			if (0 != m_redisContext->err)
			{
				printf("redis connect failed:%s\n", m_redisContext->errstr);
				return false;
			}

			m_connected = true;

			return true;
		}


		void DBRedis::DisConnect()
		{
			redisFree(m_redisContext);
			m_redisContext = NULL;
			m_connected = false;
		}


		int DBRedis::Query(const std::string& cmd, DBResultBase* result)
		{
			if (!m_connected || !result)
				return -1;

			redisReply* pReply = (redisReply*)redisCommand(m_redisContext, cmd.c_str());
			if (!pReply)
				return -1;

			if (!QueryResult(pReply, static_cast<DBRedisResult*>(result)))
				return -1;

			return 0;
		}


		int DBRedis::Ping()
		{
			DBRedisResult result;
			return Query("ping", &result);
		}


		bool DBRedis::QueryResult(redisReply* reply, DBRedisResult* result)
		{
			assert(reply && result);

			if (REDIS_REPLY_INTEGER == reply->type)
			{
				result->SetInt(reply->integer);
			}
			else if (REDIS_REPLY_STRING == reply->type || REDIS_REPLY_STATUS == reply->type || REDIS_REPLY_DOUBLE == reply->type
				|| REDIS_REPLY_BOOL == reply->type)
			{
				result->SetString(reply->str, static_cast<int>(reply->len));
			}
			else if (REDIS_REPLY_ARRAY == reply->type || REDIS_REPLY_MAP == reply->type || REDIS_REPLY_SET == reply->type)
			{
				for (int i = 0; i < reply->elements; ++i)
				{
					DBRedisResult redisResult;
					QueryResult(reply->element[i], &redisResult);
					result->AppendAry(redisResult);
				}
			}
			else if (REDIS_REPLY_NIL == reply->type)
			{
				//空返回不执行任何操作
				return true;
			}
			//错误信息
			else if (REDIS_REPLY_ERROR == reply->type)
			{
				result->SetString(reply->str, static_cast<int>(reply->len));
				return false;
			}
			else
			{			
				printf("unjudge reply type:%d\n", reply->type);
				return false;
			}

			return true;
		}

	}	//namespace db
}	// namespace chaos