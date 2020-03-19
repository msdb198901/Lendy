#ifndef LOGON_DATABASE_H
#define LOGON_DATABASE_H

#include "MySQLConnection.h"

//µÇÂ¼·þÎñÔ¤Éè
enum LogonDatabaseStatements
{
	LOGON_SEL_LIMIT_ADDRESS,
	LOGON_UPD_LIMIT_ADDRESS,
	LOGON_SEL_GAME_ID,
	LOGON_UPD_GAME_ID,
	LOGON_SEL_VISITOR_ACCOUNT,
	LOGON_INS_VISITOR_ACCOUNT,
	LOGON_UPD_VISITOR_ACCOUNT,
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