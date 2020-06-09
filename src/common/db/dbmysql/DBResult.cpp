#include "DBMysql.h"


const Field DBResult::s_nullfield;

DBResult::DBResult()
{
	m_result.reserve(DEFAULT_RESERVER);
}


DBResult::~DBResult()
{
	Clear();
}


const Field& DBResult::GetFieldByName(uint32 row, const std::string& name) const
{
	if (m_result.size() <= row)
		return s_nullfield;

	auto& rowData = m_result[row];

	int idx = Name2Index(name);
	if (0 > idx || rowData.size() <= idx)
		return s_nullfield;

	return *rowData[idx];
}


int DBResult::AppendField(uint64 row, uint32 field, const char* value, uint32 len)
{
	if (m_result.size() <= row)
	{
		std::vector<Field*> newRow;
		m_result.push_back(newRow);
	}

	auto& rowData = m_result.back();

	//已存在的列
	if (0 > field || rowData.size() > field)
		return -1;

	Field* pField = new Field();
	pField->Fill(value, len);

	rowData.push_back(pField);

	return 0;
}


bool DBResult::BuildName2Field(const std::string& name, const FieldInfo& field)
{
	m_name2info.insert(std::make_pair(name, field));

	return true;
}


void DBResult::Clear()
{
	if (m_result.empty())
		return;

	for (auto it = m_result.begin(); it != m_result.end(); ++it)
	{
		for (auto i = it->begin(); i != it->end(); ++i)
		{
			delete *i;
		}

		it->clear();
	}

	m_result.clear();
}
