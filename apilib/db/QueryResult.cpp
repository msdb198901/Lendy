#include "QueryResult.h"
#include "Field.h"
#include "Log.h"
#include <mysql.h>

namespace DB
{
	using namespace LogComm;

	DatabaseFieldTypes MysqlTypeToFieldType(enum_field_types type)
	{
		switch (type)
		{
		case MYSQL_TYPE_NULL:
			return DatabaseFieldTypes::Null;
		case MYSQL_TYPE_TINY:
			return DatabaseFieldTypes::Int8;
		case MYSQL_TYPE_YEAR:
		case MYSQL_TYPE_SHORT:
			return DatabaseFieldTypes::Int16;
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
			return DatabaseFieldTypes::Int32;
		case MYSQL_TYPE_LONGLONG:
		case MYSQL_TYPE_BIT:
			return DatabaseFieldTypes::Int64;
		case MYSQL_TYPE_FLOAT:
			return DatabaseFieldTypes::Float;
		case MYSQL_TYPE_DOUBLE:
			return DatabaseFieldTypes::Double;
		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_NEWDECIMAL:
			return DatabaseFieldTypes::Decimal;
		case MYSQL_TYPE_TIMESTAMP:
		case MYSQL_TYPE_DATE:
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_DATETIME:
			return DatabaseFieldTypes::Date;
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
			return DatabaseFieldTypes::Binary;
		default:
			LOG_WARN("sql.sql", "MysqlTypeToFieldType(): invalid field type %u", uint32(type));
			break;
		}

		return DatabaseFieldTypes::Null;
	}

	static uint32 SizeForType(MYSQL_FIELD* field)
	{
		switch (field->type)
		{
		case MYSQL_TYPE_NULL:
			return 0;
		case MYSQL_TYPE_TINY:
			return 1;
		case MYSQL_TYPE_YEAR:
		case MYSQL_TYPE_SHORT:
			return 2;
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_FLOAT:
			return 4;
		case MYSQL_TYPE_DOUBLE:
		case MYSQL_TYPE_LONGLONG:
		case MYSQL_TYPE_BIT:
			return 8;

		case MYSQL_TYPE_TIMESTAMP:
		case MYSQL_TYPE_DATE:
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_DATETIME:
			return sizeof(MYSQL_TIME);

		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
			return field->max_length + 1;

		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_NEWDECIMAL:
			return 64;

		case MYSQL_TYPE_GEOMETRY:
			/*
			Following types are not sent over the wire:
			MYSQL_TYPE_ENUM:
			MYSQL_TYPE_SET:
			*/
		default:
			LOG_WARN("sql.sql", "SQL::SizeForType(): invalid field type %u", uint32(field->type));
			return 0;
		}
	}

	///////////////////////////
	ResultSet::ResultSet(MYSQL_RES * result, MYSQL_FIELD * fields, uint64 rowCount, uint32 fieldCount):
		m_rowCount(rowCount),
		m_fieldCount(fieldCount),
		m_result(result),
		m_fields(fields)
	{
		m_currentRow = new Field[m_fieldCount];
#ifdef STRICT_DATABASE_TYPE_CHECKS
		for (uint32 i = 0; i < _fieldCount; i++)
			_currentRow[i].SetMetadata(&_fields[i], i);
#endif
	}

	ResultSet::~ResultSet()
	{
		CleanUp();
	}
	bool ResultSet::NextRow()
	{
		MYSQL_ROW row;

		if (!m_result)
		{
			return false;
		}

		row = mysql_fetch_row(m_result);
		if (!row)
		{
			CleanUp();
			return false;
		}

		unsigned long* lengths = mysql_fetch_lengths(m_result);
		if (!lengths)
		{
			LOG_WARN("sql.sql", "%s:mysql_fetch_lengths, cannot retrieve value lengths. Error %s.", __FUNCTION__, mysql_error(m_result->handle));
			CleanUp();
			return false;
		}

		for (uint32 i = 0; i < m_fieldCount; i++)
		{
			m_currentRow[i].SetStructuredValue(row[i], MysqlTypeToFieldType(m_fields[i].type), lengths[i]);
		}
		return true;
	}
	Field const & ResultSet::operator[](std::size_t index) const
	{
		assert(index < m_fieldCount);
		return m_currentRow[index];
	}
	void ResultSet::CleanUp()
	{
		if (m_currentRow)
		{
			ADELETE(m_currentRow);
		}

		if (m_result)
		{
			mysql_free_result(m_result);
			m_result = nullptr;
		}
	}

	PreparedResultSet::PreparedResultSet(MYSQL_STMT * stmt, MYSQL_RES * result, uint64 rowCount, uint32 fieldCount):
		m_rowCount(rowCount),
		m_rowPosition(0),
		m_fieldCount(fieldCount),
		m_bind(nullptr),
		m_stmt(stmt),
		m_result(result)
	{
		if (!m_result)
		{
			return;
		}

		if (m_stmt->bind_result_done)
		{
			delete[] m_stmt->bind->length;
			delete[] m_stmt->bind->is_null;
		}

		m_bind = new MYSQL_BIND[m_fieldCount];

		my_bool* m_isNull = new my_bool[m_fieldCount];

		ulong* m_length = new ulong[m_fieldCount];

		memset(m_isNull, 0, sizeof(my_bool) * m_fieldCount);
		memset(m_bind, 0, sizeof(MYSQL_BIND) * m_fieldCount);
		memset(m_length, 0, sizeof(ulong) * m_fieldCount);

		if (mysql_stmt_store_result(m_stmt))
		{
			LOG_WARN("sql.sql", "%s:mysql_stmt_store_result, cannot bind result from MySQL server. Error: %s", __FUNCTION__, mysql_stmt_error(m_stmt));
			delete[] m_bind;
			delete[] m_isNull;
			delete[] m_length;
			return;
		}

		m_rowCount = mysql_stmt_num_rows(m_stmt);

		MYSQL_FIELD* field = mysql_fetch_fields(m_result);
		std::size_t rowSize = 0;
		for (uint32 i = 0; i < m_fieldCount; ++i)
		{
			uint32 size = SizeForType(&field[i]);
			rowSize += size;

			m_bind[i].buffer_type = field[i].type;
			m_bind[i].buffer_length = size;
			m_bind[i].length = &m_length[i];
			m_bind[i].is_null = &m_isNull[i];
			m_bind[i].error = nullptr;
			m_bind[i].is_unsigned = field[i].flags & UNSIGNED_FLAG;
		}

		char* dataBuffer = new char[rowSize * m_rowCount];
		for (uint32 i = 0, offset = 0; i < m_fieldCount; ++i)
		{
			m_bind[i].buffer = dataBuffer + offset;
			offset += m_bind[i].buffer_length;
		}

		if (mysql_stmt_bind_result(m_stmt, m_bind))
		{
			LOG_WARN("sql.sql", "%s:mysql_stmt_bind_result, cannot bind result from MySQL server. Error: %s", __FUNCTION__, mysql_stmt_error(m_stmt));
			mysql_stmt_free_result(m_stmt);
			CleanUp();
			delete[] m_isNull;
			delete[] m_length;
			return;
		}

		m_vRows.resize(uint32(m_rowCount) * m_fieldCount);
		while (_NextRow())
		{
			for (uint32 fIndex = 0; fIndex < m_fieldCount; ++fIndex)
			{
				ulong buffer_length = m_bind[fIndex].buffer_length;
				ulong fetched_length = *m_bind[fIndex].length;
				if (!*m_bind[fIndex].is_null)
				{
					void* buffer = m_stmt->bind[fIndex].buffer;
					switch (m_bind[fIndex].buffer_type)
					{
					case MYSQL_TYPE_TINY_BLOB:
					case MYSQL_TYPE_MEDIUM_BLOB:
					case MYSQL_TYPE_LONG_BLOB:
					case MYSQL_TYPE_BLOB:
					case MYSQL_TYPE_STRING:
					case MYSQL_TYPE_VAR_STRING:
						if (fetched_length < buffer_length)
							*((char*)buffer + fetched_length) = '\0';
						break;
					default:
						break;
					}

					m_vRows[uint32(m_rowPosition) * m_fieldCount + fIndex].SetByteValue(
						buffer,
						MysqlTypeToFieldType(m_bind[fIndex].buffer_type),
						fetched_length);

					m_stmt->bind[fIndex].buffer = (char*)buffer + rowSize;
				}
				else
				{
					m_vRows[uint32(m_rowPosition) * m_fieldCount + fIndex].SetByteValue(
						nullptr,
						MysqlTypeToFieldType(m_bind[fIndex].buffer_type),
						*m_bind[fIndex].length);
				}

#ifdef STRICT_DATABASE_TYPE_CHECKS
				m_rows[uint32(m_rowPosition) * m_fieldCount + fIndex].SetMetadata(&field[fIndex], fIndex);
#endif
			}
			m_rowPosition++;
		}
		m_rowPosition = 0;
		mysql_stmt_free_result(m_stmt);
	}
	PreparedResultSet::~PreparedResultSet()
	{
		CleanUp();
	}
	bool PreparedResultSet::NextRow()
	{
		if (++m_rowPosition >= m_rowCount)
		{
			return false;
		}
		return true;
	}
	Field * PreparedResultSet::Fetch() const
	{
		assert(m_rowPosition < m_rowCount);
		return const_cast<Field*>(&m_vRows[uint32(m_rowPosition) * m_fieldCount]);
	}
	Field const & PreparedResultSet::operator[](std::size_t index) const
	{
		assert(m_rowPosition < m_rowCount);
		assert(index < m_fieldCount);
		return m_vRows[uint32(m_rowPosition) * m_fieldCount + index];
	}
	void PreparedResultSet::CleanUp()
	{
		if (m_result)
		{
			mysql_free_result(m_result);
		}

		if (m_bind)
		{
			delete[](char*)m_bind->buffer;
			PDELETE(m_bind);
		}
	}
	bool PreparedResultSet::_NextRow()
	{
		if (m_rowPosition >= m_rowCount)
		{
			return false;
		}

		int retval = mysql_stmt_fetch(m_stmt);
		return retval == 0 || retval == MYSQL_DATA_TRUNCATED;
	}
}