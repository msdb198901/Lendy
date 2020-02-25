#ifndef QUERY_RESULT_H
#define	QUERY_RESULT_H

#include "Define.h"
#include "DBEnvHeader.h"
#include <vector>

namespace DB
{
	class ResultSet
	{
	public:
		ResultSet(MYSQL_RES* result, MYSQL_FIELD* fields, uint64 rowCount, uint32 fieldCount);
		virtual ~ResultSet();

		bool NextRow();
		uint64 GetRowCount() const { return m_rowCount; }
		uint32 GetFieldCount() const { return m_fieldCount;}

		Field* Fetch() const { return m_currentRow; }
		Field const& operator[](std::size_t index) const;

	protected:
		uint64 m_rowCount;
		Field* m_currentRow;
		uint32 m_fieldCount;

	private:
		void CleanUp();
		MYSQL_RES* m_result;
		MYSQL_FIELD* m_fields;

		DELETE_COPY_ASSIGN(ResultSet);
	};

	class LENDY_COMMON_API PreparedResultSet
	{
	public:
		PreparedResultSet(MYSQL_STMT* stmt, MYSQL_RES* result, uint64 rowCount, uint32 fieldCount);
		virtual ~PreparedResultSet();

		bool NextRow();
		uint64 GetRowCount() const { return m_rowCount; }
		uint32 GetFieldCount() const { return m_fieldCount; }

		Field* Fetch() const;
		Field const& operator[](std::size_t index) const;

	protected:
		EXPORT_BEGIN
		std::vector<Field> m_vRows;
		EXPORT_END
		uint64 m_rowCount;
		uint64 m_rowPosition;
		uint32 m_fieldCount;

	private:
		MYSQL_BIND* m_bind;
		MYSQL_STMT* m_stmt;
		MYSQL_RES* m_result;

		void CleanUp();
		bool _NextRow();

		DELETE_COPY_ASSIGN(PreparedResultSet);
	};
}

#endif