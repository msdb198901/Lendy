#ifndef MYSQL_CONNECTION_H
#define MYSQL_CONNECTION_H

#include "DBEnvHeader.h"
#include "PreparedStatement.h"
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace Util
{
	template <typename T>
	class ProducerConsumerQueue;
}

namespace DB
{
	using namespace Util;

	class DBWorker;
	class MySQLPreparedStatement;
	class SQLOperation;

	enum ConnectionFlags
	{
		CONNECTION_ASYNC = 0x01,
		CONNECTION_SYNCH = 0x02,
		CONNECTION_BOTH  = CONNECTION_ASYNC | CONNECTION_SYNCH
	};

	struct MySQLConnectionInfo
	{
		explicit MySQLConnectionInfo(std::string const& info);
		std::string user;
		std::string password;
		std::string database;
		std::string host;
		std::string portsocket;
	};

	class LENDY_COMMON_API MySQLConnection
	{
		template <class T> friend class DBWorkerPool;

		friend class PingOperation;

	public:
		MySQLConnection(MySQLConnectionInfo& connInfo);
		MySQLConnection(ProducerConsumerQueue<SQLOperation*>* queue, MySQLConnectionInfo& connInfo);
		virtual ~MySQLConnection();

		virtual uint32  Open();
		void Ping();
		void Close();

		bool PrepareStatements();
		virtual void DoPrepareStatements() = 0;

	public:
		bool Execute(char const* sql);
		bool Execute(PreparedStatement* stmt);

		ResultSet* Query(char const* sql);
		PreparedResultSet* Query(PreparedStatement* stmt);

		bool _Query(char const* sql, MYSQL_RES**  pResult, MYSQL_FIELD** pFields, uint64* pRowCount, uint32* pFieldCount);
		bool _Query(PreparedStatement* stmt, MYSQL_RES** pResult, uint64* pRowCount, uint32* pFieldCount);

		void BeginTransaction();
		void RollbackTransaction();
		void CommitTransaction();
		int ExecuteTransaction(SQLTransaction & transaction);

		uint32 GetLastError();

	protected:
		bool LockIfReady();

		void Unlock();

		MYSQL* GetHandle() { return m_mysql; }
		MySQLPreparedStatement* GetPreparedStatement(uint32 index);
		void PrepareStatement(uint32 index, std::string const& sql, ConnectionFlags flags);

	protected:
		typedef std::vector<std::unique_ptr<MySQLPreparedStatement>> PreparedStatementContainer;
		EXPORT_BEGIN
		PreparedStatementContainer	m_stmtStorage;
		EXPORT_END
		bool m_reconnecting;
		bool m_prepareError;

	private:
		bool _HandleMySQLErrno(uint32 errNo, uint8 attemps = 5);

	private:
		ProducerConsumerQueue<SQLOperation*>*	m_queue;
		MYSQL*									m_mysql;
		MySQLConnectionInfo&					m_connectionInfo;
		ConnectionFlags							m_connectionFlags;
		EXPORT_BEGIN
		std::unique_ptr<DBWorker>				m_worker;
		std::mutex								m_mutex;
		EXPORT_END

		DELETE_COPY_ASSIGN(MySQLConnection);
	};
}

#endif