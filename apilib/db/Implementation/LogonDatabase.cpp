#include "LogonDatabase.h"

namespace DB
{
	LogonDatabaseConnection::LogonDatabaseConnection(MySQLConnectionInfo & connInfo) : MySQLConnection(connInfo)
	{
	}

	LogonDatabaseConnection::LogonDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo & connInfo) : MySQLConnection(q, connInfo)
	{
	}

	LogonDatabaseConnection::~LogonDatabaseConnection()
	{
	}

	void LogonDatabaseConnection::DoPrepareStatements()
	{
		if (!m_reconnecting)
		{
			m_stmtStorage.resize(LOGON_MAX_STATEMENTS);
		}
		PrepareStatement(LOGON_SEL_ACCOUNT, "select * from  account", CONNECTION_SYNCH);
	}
}