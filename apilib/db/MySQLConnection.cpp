#include "MySQLConnection.h"
#include "DBWorker.h"
#include "QueryResult.h"
#include "StringUtility.h"
#include "Timer.h"
#include "Log.h"

#include <errmsg.h>
#include <mysql.h>
#include <mysqld_error.h>

namespace DB
{
	using namespace LogComm;

	MySQLConnectionInfo::MySQLConnectionInfo(std::string const & info)
	{
		Tokenizer tokens(info, '|');

		if (tokens.size() != 5)
		{
			return;
		}
			
		uint8 i = 0;
		host.assign(tokens[i++]);
		portsocket.assign(tokens[i++]);
		user.assign(tokens[i++]);
		password.assign(tokens[i++]);
		database.assign(tokens[i++]);
	}

	MySQLConnection::MySQLConnection(MySQLConnectionInfo & connInfo):
		m_reconnecting(false),
		m_prepareError(false),
		m_queue(nullptr),
		m_mysql(nullptr),
		m_connectionInfo(connInfo),
		m_connectionFlags(CONNECTION_SYNCH)
	{
	}

	MySQLConnection::MySQLConnection(ProducerConsumerQueue<SQLOperation*>* queue, MySQLConnectionInfo & connInfo) :
		m_reconnecting(false),
		m_prepareError(false),
		m_queue(queue),
		m_mysql(nullptr),
		m_connectionInfo(connInfo),
		m_connectionFlags(CONNECTION_ASYNC)
	{
		m_worker = std::make_unique<DBWorker>(m_queue, this);
	}

	MySQLConnection::~MySQLConnection()
	{
		Close();
	}

	uint32 MySQLConnection::Open()
	{
		MYSQL* mysqlInit;
		mysqlInit = mysql_init(nullptr);
		if (!mysqlInit)
		{
			LOG_ERROR("sql.sql", "Could not initialize Mysql connection to database `%s`", m_connectionInfo.database.c_str());
			return CR_UNKNOWN_ERROR;
		}

		int port;
		char const* unixSocket;
		mysql_options(mysqlInit, MYSQL_SET_CHARSET_NAME, "utf8");

#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
		if (m_connectionInfo.host == ".")
		{
			uint32 opt = MYSQL_PROTOCOL_PIPE;
			mysql_options(mysqlInit, MYSQL_OPT_PROTOCOL, (char const*)&opt);
			port = 0;
			unixSocket = 0;
		}
		else
		{
			port = atoi(m_connectionInfo.portsocket.c_str());
			unixSocket = 0;
		}
#else
		if (m_connectionInfo.host == ".")
		{
			uint32 opt = MYSQL_PROTOCOL_SOCKET;
			mysql_options(mysqlInit, MYSQL_OPT_PROTOCOL, (char const*)&opt);
			m_connectionInfo.host = "localhost";
			port = 0;
			unixSocket = m_connectionInfo.portsocket.c_str();
		}
		else
		{
			port = atoi(m_connectionInfo.portsocket.c_str());
			unixSocket = nullptr;
		}
#endif
		m_mysql = mysql_real_connect(mysqlInit,
			m_connectionInfo.host.c_str(),
			m_connectionInfo.user.c_str(),
			m_connectionInfo.password.c_str(),
			m_connectionInfo.database.c_str(),
			port,
			unixSocket,
			0);

		if (m_mysql)
		{
			if (!m_reconnecting)
			{
				LOG_INFO("sql.sql", "MySQL client library: %s", mysql_get_client_info());
				LOG_INFO("sql.sql", "MySQL server ver: %s ", mysql_get_server_info(m_mysql));
			}
			LOG_INFO("sql.sql", "Connected to MySQL database at %s", m_connectionInfo.host.c_str());
			mysql_autocommit(m_mysql, 1);

			mysql_set_character_set(m_mysql, "utf8");
			return 0;
		}
		else
		{
			LOG_ERROR("sql.sql", "Could not connect to MySQL database at %s: %s", m_connectionInfo.host.c_str(), mysql_error(mysqlInit));
			uint32 errorCode = mysql_errno(mysqlInit);
			mysql_close(mysqlInit);
			return errorCode;
		}
	}

	void MySQLConnection::Ping()
	{
	}

	void MySQLConnection::Close()
	{
		m_worker.reset();
		m_stmtStorage.clear();

		if (m_mysql)
		{
			mysql_close(m_mysql);
			m_mysql = nullptr;
		}
	}

	bool MySQLConnection::PrepareStatements()
	{
		DoPrepareStatements();
		return !m_prepareError;
	}

	bool MySQLConnection::Execute(char const * sql)
	{
		if (!m_mysql)
		{
			return false;
		}
		uint32 _st= getMSTime();
		if (mysql_query(m_mysql, sql))
		{
			uint32 lErrno = mysql_errno(m_mysql);
			LOG_INFO("sql.sql", "SQL: %s", sql);
			LOG_ERROR("sql.sql", "[%u] %s", lErrno, mysql_error(m_mysql));

			if (_HandleMySQLErrno(lErrno))
			{
				return Execute(sql);
			}
			return false;
		}
		else
		{
			LOG_DEBUG("sql.sql", "[%u ms] SQL: %s", getMSTimeDiff(_st, getMSTime()), sql);
		}
		return true;
	}
	bool MySQLConnection::Execute(PreparedStatement* stmt)
	{
		if (!m_mysql)
		{
			return false;
		}
		uint32 index = stmt->m_index;

		MySQLPreparedStatement* _mpStmt = GetPreparedStatement(index);
		assert(_mpStmt);
		_mpStmt->m_pStmt = stmt;
		stmt->BindParameters(_mpStmt);

		MYSQL_STMT* msql_stmt = _mpStmt->GetSTMT();
		MYSQL_BIND* msql_bind = _mpStmt->GetBind();

		uint32 _st = getMSTime();

		if (mysql_stmt_bind_param(msql_stmt, msql_bind))
		{
			uint32 lErrno = mysql_errno(m_mysql);
			LOG_ERROR("sql.sql", "SQL(p): %s\n [ERROR]: [%u] %s", _mpStmt->GetQueryString().c_str(), lErrno, mysql_stmt_error(msql_stmt));

			if (_HandleMySQLErrno(lErrno))
			{
				return Execute(stmt);
			}

			_mpStmt->ClearParameters();
			return false;
		}

		if (mysql_stmt_execute(msql_stmt))
		{
			uint32 lErrno = mysql_errno(m_mysql);
			LOG_ERROR("sql.sql", "SQL(p): %s\n [ERROR]: [%u] %s", _mpStmt->GetQueryString().c_str(), lErrno, mysql_stmt_error(msql_stmt));

			if (_HandleMySQLErrno(lErrno))
			{
				return Execute(stmt);
			}
		
			_mpStmt->ClearParameters();
			return false;
		}

		LOG_DEBUG("sql.sql", "[%u ms] SQL(p): %s", getMSTimeDiff(_st, getMSTime()), _mpStmt->GetQueryString().c_str());
		_mpStmt->ClearParameters();
		return true;
	}

	ResultSet * MySQLConnection::Query(char const * sql)
	{
		if (!sql)
		{
			return nullptr;
		}

		MYSQL_RES *result = nullptr;
		MYSQL_FIELD *fields = nullptr;
		uint64 rowCount = 0;
		uint32 fieldCount = 0;

		if (!_Query(sql, &result, &fields, &rowCount, &fieldCount))
		{
			return nullptr;
		}
		return new ResultSet(result, fields, rowCount, fieldCount);
	}

	PreparedResultSet * MySQLConnection::Query(PreparedStatement * stmt)
	{
		MYSQL_RES *result = nullptr;
		uint64 rowCount = 0;
		uint32 fieldCount = 0;

		if (!_Query(stmt, &result, &rowCount, &fieldCount))
			return nullptr;

		if (mysql_more_results(m_mysql))
		{
			mysql_next_result(m_mysql);
		}
		return new PreparedResultSet(stmt->m_mpStmt->GetSTMT(), result, rowCount, fieldCount);
	}

	bool MySQLConnection::_Query(char const * sql, MYSQL_RES ** pResult, MYSQL_FIELD ** pFields, uint64 * pRowCount, uint32 * pFieldCount)
	{
		if (!m_mysql)
			return false;

		{
			uint32 _s = getMSTime();

			if (mysql_query(m_mysql, sql))
			{
				uint32 lErrno = mysql_errno(m_mysql);
				LOG_INFO("sql.sql", "SQL: %s", sql);
				LOG_ERROR("sql.sql", "[%u] %s", lErrno, mysql_error(m_mysql));

				if (_HandleMySQLErrno(lErrno))      // If it returns true, an error was handled successfully (i.e. reconnection)
					return _Query(sql, pResult, pFields, pRowCount, pFieldCount);    // We try again

				return false;
			}
			else
				LOG_DEBUG("sql.sql", "[%u ms] SQL: %s", getMSTimeDiff(_s, getMSTime()), sql);

			*pResult = mysql_store_result(m_mysql);
			*pRowCount = mysql_affected_rows(m_mysql);
			*pFieldCount = mysql_field_count(m_mysql);
		}

		if (!*pResult)
			return false;

		if (!*pRowCount)
		{
			mysql_free_result(*pResult);
			return false;
		}

		*pFields = mysql_fetch_fields(*pResult);

		return true;
	}

	bool MySQLConnection::_Query(PreparedStatement * stmt, MYSQL_RES ** pResult, uint64 * pRowCount, uint32 * pFieldCount)
	{
		if (!m_mysql)
			return false;

		uint32 index = stmt->m_index;

		MySQLPreparedStatement* m_mpStmt = GetPreparedStatement(index);
		assert(m_mpStmt);            // Can only be null if preparation failed, server side error or bad query
		m_mpStmt->m_pStmt = stmt;     // Cross reference them for debug output

		stmt->BindParameters(m_mpStmt);

		MYSQL_STMT* msql_STMT = m_mpStmt->GetSTMT();
		MYSQL_BIND* msql_BIND = m_mpStmt->GetBind();

		uint32 _s = getMSTime();

		if (mysql_stmt_bind_param(msql_STMT, msql_BIND))
		{
			uint32 lErrno = mysql_errno(m_mysql);
			LOG_ERROR("sql.sql", "SQL(p): %s\n [ERROR]: [%u] %s", m_mpStmt->GetQueryString().c_str(), lErrno, mysql_stmt_error(msql_STMT));

			if (_HandleMySQLErrno(lErrno))  // If it returns true, an error was handled successfully (i.e. reconnection)
				return _Query(stmt, pResult, pRowCount, pFieldCount);       // Try again

			m_mpStmt->ClearParameters();
			return false;
		}

		if (mysql_stmt_execute(msql_STMT))
		{
			uint32 lErrno = mysql_errno(m_mysql);
			LOG_ERROR("sql.sql", "SQL(p): %s\n [ERROR]: [%u] %s",
				m_mpStmt->GetQueryString().c_str(), lErrno, mysql_stmt_error(msql_STMT));

			if (_HandleMySQLErrno(lErrno))  // If it returns true, an error was handled successfully (i.e. reconnection)
				return _Query(stmt, pResult, pRowCount, pFieldCount);      // Try again

			m_mpStmt->ClearParameters();
			return false;
		}

		LOG_DEBUG("sql.sql", "[%u ms] SQL(p): %s", getMSTimeDiff(_s, getMSTime()), m_mpStmt->GetQueryString().c_str());

		m_mpStmt->ClearParameters();

		*pResult = mysql_stmt_result_metadata(msql_STMT);
		*pRowCount = mysql_stmt_num_rows(msql_STMT);
		*pFieldCount = mysql_stmt_field_count(msql_STMT);

		return true;
	}

	void MySQLConnection::BeginTransaction()
	{
	}

	void MySQLConnection::RollbackTransaction()
	{
	}

	void MySQLConnection::CommitTransaction()
	{
	}

	int MySQLConnection::ExecuteTransaction(SQLTransaction & transaction)
	{
		return 0;
	}

	uint32 MySQLConnection::GetLastError()
	{
		return mysql_errno(m_mysql);
	}

	bool MySQLConnection::LockIfReady()
	{
		return m_mutex.try_lock();
	}

	void MySQLConnection::Unlock()
	{
		m_mutex.unlock();
	}

	MySQLPreparedStatement * MySQLConnection::GetPreparedStatement(uint32 index)
	{
		assert(index < m_stmtStorage.size());
		MySQLPreparedStatement* ret = m_stmtStorage[index].get();
		if (!ret)
		{
			LOG_ERROR("sql.sql", "Could not fetch prepared statement %u on database `%s`, connection type: %s.",
				index, m_connectionInfo.database.c_str(), (m_connectionFlags & CONNECTION_ASYNC) ? "asynchronous" : "synchronous");
		}
		return ret;
	}

	void MySQLConnection::PrepareStatement(uint32 index, std::string const & sql, ConnectionFlags flags)
	{
		if (!(m_connectionFlags & flags))
		{
			m_stmtStorage[index].reset();
			return;
		}
		MYSQL_STMT* stmt = mysql_stmt_init(m_mysql);
		if (!stmt)
		{
			LOG_ERROR("sql.sql", "In mysql_stmt_init() id: %u, sql: \"%s\"", index, sql.c_str());
			LOG_ERROR("sql.sql", "%s", mysql_error(m_mysql));
			m_prepareError = true;
		}
		else
		{
			if (mysql_stmt_prepare(stmt, sql.c_str(), static_cast<unsigned long>(sql.size())))
			{
				LOG_ERROR("sql.sql", "In mysql_stmt_prepare() id: %u, sql: \"%s\"", index, sql.c_str());
				LOG_ERROR("sql.sql", "%s", mysql_stmt_error(stmt));
				mysql_stmt_close(stmt);
				m_prepareError = true;
			}
			else
			{
				m_stmtStorage[index] = std::make_unique<MySQLPreparedStatement>(stmt, sql);
			}
		}
	}

	bool MySQLConnection::_HandleMySQLErrno(uint32 errNo, uint8 attempts)
	{
		switch (errNo)
		{
		case CR_SERVER_GONE_ERROR:
		case CR_SERVER_LOST:
		case CR_SERVER_LOST_EXTENDED:
		{
			if (m_mysql)
			{
				LOG_ERROR("sql.sql", "Lost the connection to the MySQL server!");

				mysql_close(GetHandle());
				m_mysql = nullptr;
			}
		}
		case CR_CONN_HOST_ERROR:
		{
			LOG_INFO("sql.sql", "Attempting to reconnect to the MySQL server...");

			m_reconnecting = true;

			uint32 const lErrno = Open();
			if (!lErrno)
			{
				if (!this->PrepareStatements())
				{
					LOG_FATAL("sql.sql", "Could not re-prepare statements!");
					std::this_thread::sleep_for(std::chrono::seconds(10));
					std::abort();
				}

				LOG_INFO("sql.sql", "Successfully reconnected to %s @%s:%s (%s).",
					m_connectionInfo.database.c_str(), m_connectionInfo.host.c_str(), m_connectionInfo.portsocket.c_str(),
					(m_connectionFlags & CONNECTION_ASYNC) ? "asynchronous" : "synchronous");

				m_reconnecting = false;
				return true;
			}

			if ((--attempts) == 0)
			{
				LOG_FATAL("sql.sql", "Failed to reconnect to the MySQL server, "
					"terminating the server to prevent data corruption!");

				std::this_thread::sleep_for(std::chrono::seconds(10));
				std::abort();
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::seconds(3));
				return _HandleMySQLErrno(lErrno, attempts); 
			}
		}

		case ER_LOCK_DEADLOCK:
			return false;  

		case ER_WRONG_VALUE_COUNT:
		case ER_DUP_ENTRY:
			return false;

		case ER_BAD_FIELD_ERROR:
		case ER_NO_SUCH_TABLE:
			LOG_ERROR("sql.sql", "Your database structure is not up to date. Please make sure you've executed all queries in the sql/updates folders.");
			std::this_thread::sleep_for(std::chrono::seconds(10));
			std::abort();
			return false;
		case ER_PARSE_ERROR:
			LOG_ERROR("sql.sql", "Error while parsing SQL. Core fix required.");
			std::this_thread::sleep_for(std::chrono::seconds(10));
			std::abort();
			return false;
		default:
			LOG_ERROR("sql.sql", "Unhandled MySQL errno %u. Unexpected behaviour possible.", errNo);
			return false;
		}
	}
}