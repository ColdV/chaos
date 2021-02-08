#include "DBResult.h"


namespace chaos
{
	namespace db
	{
		DBResultBase::~DBResultBase() 
		{}


		DBMysqlResult::DBMysqlResult()
		{
			m_result.reserve(DEFAULT_RESERVER);
		}


		DBMysqlResult::~DBMysqlResult()
		{
		}


		const DBMysqlResult::Field* DBMysqlResult::GetFieldByName(uint32 row, const std::string& name) const
		{
			if (m_result.size() <= row)
				return NULL;

			auto& rowData = m_result[row];

			int idx = Name2Index(name);
			if (0 > idx || rowData.size() <= idx)
				return NULL;

			return &rowData[idx];
		}


		int DBMysqlResult::AppendField(uint64 row, uint32 field, const char* value, uint32 len)
		{
			if (m_result.size() <= row)
			{
				std::vector<Field> newRow;
				m_result.push_back(newRow);
			}

			auto& rowData = m_result.back();

			//已存在的列
			if (0 > field || rowData.size() > field)
				return -1;

			Field oField;

			oField.Fill(value, len);

			rowData.push_back(oField);

			return 0;
		}


		bool DBMysqlResult::BuildName2Field(const std::string& name, const FieldInfo& field)
		{
			m_name2info.insert(std::make_pair(name, field));

			return true;
		}

	}	//namespace db
}	//namespace chaos