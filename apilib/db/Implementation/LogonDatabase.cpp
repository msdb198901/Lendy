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
		
		PrepareStatement(LOGON_SEL_GAME_ID, "SELECT gameid FROM platform_id_mgr", CONNECTION_SYNCH);
		PrepareStatement(LOGON_UPD_GAME_ID, "UPDATE platform_id_mgr SET gameid=gameid+1", CONNECTION_SYNCH);
		
		PrepareStatement(LOGON_SEL_VISITOR_ACCOUNT, "SELECT id,account,username,sha_pass_hash,face_url,service_limit FROM account_info WHERE register_machine = ?", CONNECTION_SYNCH);
		PrepareStatement(LOGON_INS_VISITOR_ACCOUNT, "INSERT INTO account_info(account,username,sha_pass_hash,face_url,logon_type,register_ip,register_machine) VALUES (?,?,?,?,?,?,?)", CONNECTION_SYNCH);
		PrepareStatement(LOGON_UPD_VISITOR_ACCOUNT, "UPDATE account_info SET last_logon_ip=?,last_logon_date=NOW() WHERE register_machine = ?", CONNECTION_SYNCH);
	}
}