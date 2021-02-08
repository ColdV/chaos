#pragma once

#include "stdafx.h"


namespace chaos
{
	namespace db
	{
		class DBResultBase
		{
		public:
			DBResultBase() {}
			virtual ~DBResultBase() = 0;
		};


#ifdef DB_MYSQL_ENABLE
		class DBMysqlResult : public DBResultBase
		{
		public:
			class Field;
			static const int DEFAULT_RESERVER = 128;		//默认预分配大小
			static const Field s_nullfield;				//空Field对象
			
			friend class DBMysql;

			struct FieldInfo
			{
				std::string name;		//列名
				uint32 index;			//列下标
			};

			typedef std::vector<std::vector<Field>>	DBResultVec;
			typedef std::map<std::string, FieldInfo>	Name2FieldInfoMap;

			DBMysqlResult();
			~DBMysqlResult();

			//获取结果集行数
			uint32 GetRowCount() const { return m_result.size(); }

			//根据指定行数和列名获取列
			const Field* GetFieldByName(uint32 row, const std::string& name) const;

		private:
			//在指定行中添加列数据
			//@parma row:行标
			//@param field:列标
			//@param value:列值
			//@parma len:列值长度
			int AppendField(uint64 row, uint32 field, const char* value, uint32 len);

			//构建列名与列的关系
			bool BuildName2Field(const std::string& name, const FieldInfo& field);

			//根据列名获取列下标
			int Name2Index(const std::string& name) const
			{
				auto it = m_name2info.find(name);

				return it == m_name2info.end() ? -1 : it->second.index;
			}

		private:
			DBResultVec m_result;
			Name2FieldInfoMap m_name2info;
		};



		class DBMysqlResult::Field
		{
		public:
			static const uint32 MAX_SIZE = 1024 * 1024 * 10;	//最大容量(10M)

			Field() {}

			~Field() {}

			//填充列数据
			int Fill(const char* value, uint32 len)
			{
				if (MAX_SIZE <= len || 0 != m_value.size())
					return -1;

				m_value.reserve(len);

				memcpy(&m_value[0], value, len);

				return 0;
			}


			/////////////////////数据类型转换/////////////////////

			float Float() const { return Value() ? (float)atof(Value()) : 0; }

			double Double() const { return Value() ? atof(Value()) : 0; }

			bool Bool() const { return Value() ? atoi(Value()) != 0 : false; }

			uint8 Uint8() const { return Value() ? (uint8)atoi(Value()) : 0; }

			int8 Int8() const { return Value() ? (int8)atoi(Value()) : 0; }

			uint16 Uint16() const { return Value() ? (uint16)atoi(Value()) : 0; }

			int16 Int16() const { return Value() ? (int16)atoi(Value()) : 0; }

			uint32 Uint32() const { return Value() ? (uint32)strtoul(Value(), NULL, 10) : 0; }

			int32 Int32() const { return Value() ? (int32)atoi(Value()) : 0; }

			uint64 Uint64() const { return Value() ? strtoull(Value(), NULL, 10) : 0; }

			int64 Int64() const { return Value() ? atoll(Value()) : 0; }

			const char* String() const { return Value(); }

			uint32 Length() const { return m_value.size(); }

		private:
			const char* Value() const { return m_value.empty() ? NULL : &m_value[0]; }

		private:
			std::vector<char> m_value;
		};

#endif // DB_MYSQL_ENABLE



#ifdef DB_REDIS_ENABLE

		class DBRedisResult : public DBResultBase
		{
		public:
			friend class DBRedis;

			DBRedisResult() :m_int(0) {}
			~DBRedisResult() {}

			int64 Int() const { return m_int; }

			const std::string& String() const { return m_str; }

			const std::vector<DBRedisResult>& Ary() const {return m_ary; }


		private:
			void SetInt(int64 value) { m_int = value; }

			void SetString(const char* value, int len) { m_str.insert(0, value, len); }

			void AppendAry(const DBRedisResult& member) { m_ary.push_back(member); }

		private:
			int64 m_int;
			std::string m_str;
			std::vector<DBRedisResult> m_ary;
		};

#endif // DB_REDIS_ENABLE

	}	//namespace db
}	//namespace chaos