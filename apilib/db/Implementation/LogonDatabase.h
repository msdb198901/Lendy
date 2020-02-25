#ifndef LOGON_DATABASE_H
#define LOGON_DATABASE_H

#include "MySQLConnection.h"

//µÇÂ¼·þÎñÔ¤Éè
enum LogonDatabaseStatements
{
	LOGON_SEL_ACCOUNT,
	LOGON_MAX_STATEMENTS
};

namespace DB
{
	class LENDY_COMMON_API LogonDatabaseConnection : public MySQLConnection
	{
	public:
		typedef LogonDatabaseStatements Statements;

		LogonDatabaseConnection(MySQLConnectionInfo& connInfo);
		LogonDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo);
		~LogonDatabaseConnection();

		void DoPrepareStatements() override;
	};
}

#endif