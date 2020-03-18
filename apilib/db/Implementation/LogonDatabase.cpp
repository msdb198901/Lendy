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
		
		PrepareStatement(LOGON_SEL_LIMIT_ADDRESS, "SELECT enjoin_logon,enjoin_reg,UNIX_TIMESTAMP(expire_date),restore FROM limit_address WHERE addr = ? and machine = ?", CONNECTION_SYNCH);
		PrepareStatement(LOGON_UPD_LIMIT_ADDRESS, "UPDATE enjoin_logon=?,enjoin_reg=?,collect_date=NOW(),restore=? FROM limit_address WHERE addr = ? and machine = ?", CONNECTION_SYNCH);
	}
}