#ifndef SQL_OPERATION_H
#define SQL_OPERATION_H

#include "DBEnvHeader.h"

namespace DB
{
	union SQLElementUnion
	{
		PreparedStatement* stmt;
		char const* query;
	};

	enum SQLElementDataType
	{
		SE_RAW,
		SE_PREPARED,
	};

	struct SQLElementData
	{
		SQLElementUnion		element;
		SQLElementDataType	type;
	};

	union SQLResultSetUnion
	{
		PreparedResultSet*	presult;
		ResultSet*			qresult;
	};

	class MySQLConnection;

	class SQLOperation
	{
	public:
		SQLOperation() : m_conn(nullptr) {}

		virtual ~SQLOperation()
		{

		}

		virtual int call()
		{
			Execute();
			return 0;
		}

		virtual bool Execute() = 0;
		virtual void SetConnection(MySQLConnection* conn) { m_conn = conn; }

		MySQLConnection *m_conn;

		DELETE_COPY_ASSIGN(SQLOperation);
	};
}

#endif