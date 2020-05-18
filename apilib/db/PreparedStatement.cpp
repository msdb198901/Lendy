#include "PreparedStatement.h"
#include "MySQLConnection.h"
#include "QueryResult.h"
#include "Log.h"

#include <mysql.h>
#include <sstream>

namespace DB
{
	using namespace LogComm;
	PreparedStatement::PreparedStatement(uint32 index, uint8 capacity):
		m_mpStmt(nullptr),
		m_index(index),
		m_stdata(capacity)
	{
	}

	PreparedStatement::~PreparedStatement()
	{
	}

	void PreparedStatement::SetNull(const uint8 index)
	{
		assert(index < m_stdata.size());
		m_stdata[index].type = PSVT_NULL;
	}
	void PreparedStatement::SetBool(const uint8 index, const bool value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.boolean = value;
		m_stdata[index].type = PSVT_BOOL;
	}

	void PreparedStatement::SetUInt8(const uint8 index, const uint8 value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.ui8 = value;
		m_stdata[index].type = PSVT_UI8;
	}

	void PreparedStatement::SetUInt16(const uint8 index, const uint16 value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.ui16 = value;
		m_stdata[index].type = PSVT_UI16;
	}

	void PreparedStatement::SetUInt32(const uint8 index, const uint32 value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.ui32 = value;
		m_stdata[index].type = PSVT_UI32;
	}

	void PreparedStatement::SetUInt64(const uint8 index, const uint64 value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.ui64 = value;
		m_stdata[index].type = PSVT_UI64;
	}

	void PreparedStatement::SetInt8(const uint8 index, const int8 value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.i8 = value;
		m_stdata[index].type = PSVT_I8;
	}

	void PreparedStatement::SetInt16(const uint8 index, const int16 value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.i16 = value;
		m_stdata[index].type = PSVT_I16;
	}

	void PreparedStatement::SetInt32(const uint8 index, const int32 value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.i32 = value;
		m_stdata[index].type = PSVT_I32;
	}

	void PreparedStatement::SetInt64(const uint8 index, const int64 value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.i64 = value;
		m_stdata[index].type = PSVT_I64;
	}

	void PreparedStatement::SetFloat(const uint8 index, const float value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.f = value;
		m_stdata[index].type = PSVT_FLOAT;
	}

	void PreparedStatement::SetDouble(const uint8 index, const double value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].data.d = value;
		m_stdata[index].type = PSVT_DOUBLE;
	}

	void PreparedStatement::SetString(const uint8 index, const std::string & value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].binary.resize(value.length()+1);
		memcpy(m_stdata[index].binary.data(), value.c_str(), value.length() + 1);
		m_stdata[index].type = PSVT_STRING;
	}

	void PreparedStatement::SetBinary(const uint8 index, const std::vector<uint8>& value)
	{
		assert(index < m_stdata.size());
		m_stdata[index].binary = value;
		m_stdata[index].type = PSVT_BINARY;
	}

	void PreparedStatement::BindParameters(MySQLPreparedStatement * stmt)
	{
		assert(stmt);

		m_mpStmt = stmt;

		uint8 i = 0;
		for (; i < m_stdata.size(); ++i)
		{
			switch (m_stdata[i].type)
			{
			case PSVT_BOOL:
				stmt->SetBool(i, m_stdata[i].data.boolean);
				break;
			case PSVT_UI8:
				stmt->SetUInt8(i, m_stdata[i].data.ui8);
				break;
			case PSVT_UI16:
				stmt->SetUInt16(i, m_stdata[i].data.ui16);
				break;
			case PSVT_UI32:
				stmt->SetUInt32(i, m_stdata[i].data.ui32);
				break;
			case PSVT_I8:
				stmt->SetInt8(i, m_stdata[i].data.i8);
				break;
			case PSVT_I16:
				stmt->SetInt16(i, m_stdata[i].data.i16);
				break;
			case PSVT_I32:
				stmt->SetInt32(i, m_stdata[i].data.i32);
				break;
			case PSVT_UI64:
				stmt->SetUInt64(i, m_stdata[i].data.ui64);
				break;
			case PSVT_I64:
				stmt->SetInt64(i, m_stdata[i].data.i64);
				break;
			case PSVT_FLOAT:
				stmt->SetFloat(i, m_stdata[i].data.f);
				break;
			case PSVT_DOUBLE:
				stmt->SetDouble(i, m_stdata[i].data.d);
				break;
			case PSVT_STRING:
				stmt->SetBinary(i, m_stdata[i].binary, true);
				break;
			case PSVT_BINARY:
				stmt->SetBinary(i, m_stdata[i].binary, false);
				break;
			case PSVT_NULL:
				stmt->SetNull(i);
				break;
			}
		}
#ifdef _DEBUG
		if (i < stmt->m_paramCount)
		{
			LOG_WARN("sql.sql", "[WARNING]: BindParameters() for statement %u did not bind all allocated parameters", m_index);
		}
#endif
	}

	static bool IndexFail(uint32 stmtIndex, uint8 index, uint32 paramCount)
	{
		LOG_ERROR("sql.driver", 
			"Attempted to bind parameter %u%s on a PreparedStatement %u (statement has only %u parameters)", 
			uint32(index) + 1, 
			(index == 1 ? "st" : (index == 2 ? "nd" : (index == 3 ? "rd" : "nd"))), 
			stmtIndex, 
			paramCount);
		return false;
	}


	static void SetParameterValue(MYSQL_BIND* param, enum_field_types type, void const* value, uint32 len, bool isUnsigned)
	{
		param->buffer_type = type;
		delete[] static_cast<char*>(param->buffer);
		param->buffer = new char[len];
		param->buffer_length = 0;
		param->is_null_value = 0;
		param->length = nullptr;
		param->is_unsigned = isUnsigned;
		memcpy(param->buffer, value, len);
	}

	MySQLPreparedStatement::MySQLPreparedStatement(MYSQL_STMT * stmt, std::string queryString):
		m_pStmt(nullptr),
		m_mStmt(stmt),
		m_bind(nullptr),
		m_queryString(std::move(queryString))
	{
		m_paramCount = mysql_stmt_param_count(stmt);
		m_paramsSet.assign(m_paramCount, false);
		m_bind = new MYSQL_BIND[m_paramCount];
		memset(m_bind, 0, sizeof(MYSQL_BIND)*m_paramCount);

		my_bool temp = 1;
		mysql_stmt_attr_set(m_mStmt, STMT_ATTR_UPDATE_MAX_LENGTH, &temp);
	}

	MySQLPreparedStatement::~MySQLPreparedStatement()
	{
		ClearParameters();
		if (m_mStmt->bind_result_done)
		{
			delete[] m_mStmt->bind->length;
			delete[] m_mStmt->bind->is_null;
		}
		mysql_stmt_close(m_mStmt);
		delete[] m_bind;
	}

	void MySQLPreparedStatement::SetNull(const uint8 index)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		param->buffer_type = MYSQL_TYPE_NULL;
		delete[] static_cast<char*>(param->buffer);
		param->buffer = nullptr;
		param->buffer_length = 0;
		param->is_null_value = 1;
		PDELETE(param->length);
	}

	void MySQLPreparedStatement::SetBool(const uint8 index, const bool value)
	{
		SetUInt8(index, value ? 1 : 0);
	}

	void MySQLPreparedStatement::SetUInt8(const uint8 index, const uint8 value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_TINY, &value, sizeof(uint8), true);
	}

	void MySQLPreparedStatement::SetUInt16(const uint8 index, const uint16 value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_SHORT, &value, sizeof(uint16), true);
	}

	void MySQLPreparedStatement::SetUInt32(const uint8 index, const uint32 value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_LONG, &value, sizeof(uint32), true);
	}
	void MySQLPreparedStatement::SetUInt64(const uint8 index, const uint64 value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_LONGLONG, &value, sizeof(uint64), true);
	}

	void MySQLPreparedStatement::SetInt8(const uint8 index, const int8 value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_TINY, &value, sizeof(int8), true);
	}

	void MySQLPreparedStatement::SetInt16(const uint8 index, const int16 value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_SHORT, &value, sizeof(uint16), true);
	}

	void MySQLPreparedStatement::SetInt32(const uint8 index, const int32 value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_LONG, &value, sizeof(int32), true);
	}

	void MySQLPreparedStatement::SetInt64(const uint8 index, const int64 value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_LONGLONG, &value, sizeof(int64), true);
	}

	void MySQLPreparedStatement::SetFloat(const uint8 index, const float value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_FLOAT, &value, sizeof(float), true);
	}

	void MySQLPreparedStatement::SetDouble(const uint8 index, const double value)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		SetParameterValue(param, MYSQL_TYPE_DOUBLE, &value, sizeof(double), true);
	}

	void MySQLPreparedStatement::SetBinary(const uint8 index, const std::vector<uint8>& value, bool isString)
	{
		AssertValidIndex(index);
		m_paramsSet[index] = true;
		MYSQL_BIND* param = &m_bind[index];
		uint32 len = uint32(value.size());
		param->buffer_type = MYSQL_TYPE_BLOB;
		
		delete[] static_cast<char*>(param->buffer);
		param->buffer = new char[len];
		param->buffer_length = 0;
		param->is_null_value = 0;

		delete param->length;
		param->length = new ulong(len);
		
		if (isString)
		{
			*param->length -= 1;
			param->buffer_type = MYSQL_TYPE_VAR_STRING;
		}
		memcpy(param->buffer, value.data(), len);
	}

	void MySQLPreparedStatement::ClearParameters()
	{
		for (uint32 i = 0; i < m_paramCount; ++i)
		{
			PDELETE(m_bind[i].length);
			delete[] static_cast<char*>(m_bind[i].buffer);
			m_bind[i].buffer = nullptr;
			m_paramsSet[i] = false;
		}
	}

	void MySQLPreparedStatement::AssertValidIndex(uint8 index)
	{
		assert(index < m_paramCount || IndexFail(m_pStmt->m_index, index, m_paramCount));
		if (m_paramsSet[index])
		{
			LOG_ERROR("sql.sql", 
				"Prepared Statement (id: %u) trying to bind value on already bound index (%u).",
				m_pStmt->m_index, 
				index);
		}
	}

	std::string MySQLPreparedStatement::GetQueryString() const
	{
		std::string queryString(m_queryString);
		
		size_t pos = 0;
		for (uint32 i = 0; i < m_pStmt->m_stdata.size(); ++i)
		{
			pos = queryString.find('?', pos);
			std::stringstream ss;

			switch (m_pStmt->m_stdata[i].type)
			{
			case PSVT_BOOL:
				ss << uint16(m_pStmt->m_stdata[i].data.boolean);
				break;
			case PSVT_UI8:
				ss << uint16(m_pStmt->m_stdata[i].data.ui8);
				break;
			case PSVT_UI16:
				ss << m_pStmt->m_stdata[i].data.ui16;
				break;
			case PSVT_UI32:
				ss << m_pStmt->m_stdata[i].data.ui32;
				break;
			case PSVT_I8:
				ss << int16(m_pStmt->m_stdata[i].data.i8);
				break;
			case PSVT_I16:
				ss << m_pStmt->m_stdata[i].data.i16;
				break;
			case PSVT_I32:
				ss << m_pStmt->m_stdata[i].data.i32;
				break;
			case PSVT_UI64:
				ss << m_pStmt->m_stdata[i].data.ui64;
				break;
			case PSVT_I64:
				ss << m_pStmt->m_stdata[i].data.i64;
				break;
			case PSVT_FLOAT:
				ss << m_pStmt->m_stdata[i].data.f;
				break;
			case PSVT_DOUBLE:
				ss << m_pStmt->m_stdata[i].data.d;
				break;
			case PSVT_STRING:
				ss << '\'' << (char const*)m_pStmt->m_stdata[i].binary.data() << '\'';
				break;
			case PSVT_BINARY:
				ss << "BINARY";
				break;
			case PSVT_NULL:
				ss << "NULL";
				break;
			}
			std::string replaceStr = ss.str();
			queryString.replace(pos, 1, replaceStr);
			pos += replaceStr.length();
		}

		return queryString;
	}


	PreparedStatementTask::PreparedStatementTask(PreparedStatement * stmt, bool async):
		m_pStmt(stmt),
		m_result(nullptr)
	{
		m_bResult = async;
		if (async)
		{
			m_result = new PreparedQueryResultPromise();
		}
	}

	PreparedStatementTask::~PreparedStatementTask()
	{
		PDELETE(m_pStmt);
		if (m_bResult && m_result != nullptr)
		{
			PDELETE(m_result);
		}
	}

	bool PreparedStatementTask::Execute()
	{
		if (m_bResult)
		{
			PreparedResultSet* result = m_conn->Query(m_pStmt);
			if (!result || !result->GetRowCount())
			{
				delete result;
				m_result->set_value(PreparedQueryResult(nullptr));
				return false;
			}
			m_result->set_value(PreparedQueryResult(result));
			return true;
		}

		return m_conn->Execute(m_pStmt);
	}
}