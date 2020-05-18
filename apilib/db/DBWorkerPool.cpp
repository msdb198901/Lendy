#include "DBWorkerPool.h"
#include "PreparedStatement.h"
#include "ProducerConsumerQueue.h"
#include "QueryCallback.h"
#include "QueryResult.h"
#include "SQLOperation.h"
#include "MySQLConnection.h"
#include "Log.h"
#include "INIReader.h"

#include "DBUpdater.h"

#include "Implementation/LogonDatabase.h"

#include <mysql.h>
#include <mysqld_error.h>

#define MIN_MYSQL_SERVER_VERSION 50100u
#define MIN_MYSQL_CLIENT_VERSION 50100u

namespace DB
{
	using namespace Util;
	using namespace LogComm;

	template<typename T>
	DBWorkerPool<T>::DBWorkerPool():
		m_queue(new ProducerConsumerQueue<SQLOperation*>())
	{
		if (!mysql_thread_safe())
		{
			LOG_ERROR("sql.driver", "Used MySQL library isn't thread-safe.");
			abort();
		}

		if (mysql_get_client_version() < MIN_MYSQL_CLIENT_VERSION)
		{
			LOG_ERROR("sql.driver", "Lendy does not support MySQL versions below 5.1");
			abort();
		}
	}

	template<typename T>
	DBWorkerPool<T>::~DBWorkerPool()
	{
		m_queue->Cancel();
	}

	template<typename T>
	void DBWorkerPool<T>::SetConnectionInfo(std::string const & infoString, uint8 const asyncThreads, uint8 const synchThreads)
	{
		m_connectionInfo = std::make_unique<MySQLConnectionInfo>(infoString);
		m_asyncThreads = asyncThreads;
		m_synchThreads = synchThreads;
	}

	template<typename T>
	uint32 DBWorkerPool<T>::Open()
	{
		if (!m_connectionInfo.get())
		{
			assert(nullptr);
			LOG_ERROR("sql.driver", "Connection info was not set!");
		}

		LOG_INFO("sql.driver", "Opening DatabasePool '%s'. "
			"Asynchronous connections: %u, synchronous connections: %u.",
			GetDatabaseName(), m_asyncThreads, m_synchThreads);

		uint32 error = OpenConnections(IDX_ASYNC, m_asyncThreads);

		if (error)
		{
			return error;
		}

		error = OpenConnections(IDX_SYNCH, m_synchThreads);

		if (!error)
		{
			LOG_INFO("sql.driver", "DatabasePool '%s' opened successfully. %d total connections running.",
				GetDatabaseName(), (m_connections[IDX_SYNCH].size() + m_connections[IDX_ASYNC].size()));
		}
		return error;
	}

	template<typename T>
	void DBWorkerPool<T>::Close()
	{
		LOG_INFO("sql.driver", "Closing down DatabasePool '%s'.", GetDatabaseName());
		m_connections[IDX_ASYNC].clear();

		LOG_INFO("sql.driver", "Asynchronous connections on DatabasePool '%s' terminated. ", GetDatabaseName());
		m_connections[IDX_SYNCH].clear();

		LOG_INFO("sql.driver", "All connections on DatabasePool '%s' closed.", GetDatabaseName());
	}

	template<typename T>
	bool DBWorkerPool<T>::PrepareStatements()
	{
		for (size_t i = 0; i < IDX_SIZE; ++i)
		{
			for (auto& item : m_connections[i])
			{
				item->LockIfReady();
				if (!item->PrepareStatements())
				{
					item->Unlock();
					//Close();
					return false;
				}
				else
				{
					item->Unlock();
				}

				size_t const preparedSize = item->m_stmtStorage.size();
				if (m_preparedStatementSize.size() < preparedSize)
				{
					m_preparedStatementSize.resize(preparedSize);
				}

				for (size_t i = 0; i < preparedSize; ++i)
				{
					if (m_preparedStatementSize[i] > 0)
					{
						continue;
					}

					if (MySQLPreparedStatement* stmt = item->m_stmtStorage[i].get())
					{
						uint32 const paramCount = stmt->GetParameterCount();

						//assert(paramCount < std::numeric_limits<uint8>::max());
						m_preparedStatementSize[i] = static_cast<uint8>(paramCount);
					}
				}
			}
		}
		return true;
	}

	template<typename T>
	bool DBWorkerPool<T>::Start(DBWorkerPool<T> &pool)
	{
		m_updateFlags = sConfigMgr->GetInt32("DB", "UpdateFlags", 0);
		bool const updatesEnabledForThis = m_updateFlags == 1;/*DBUpdater<T>::IsEnabled(_updateFlags)*/;
		std::string name = sConfigMgr->Get("DB", "Name", "");
		m_open.push([this, name, updatesEnabledForThis, &pool]() -> bool
		{
			std::string const dbString = sConfigMgr->Get("DB", "ConnectionInfo", "");
			if (dbString.empty())
			{
				LOG_ERROR("sql.driver", "Database %s not specified in configuration file!", name.c_str());
				return false;
			}

			uint8 const asyncThreads = uint8(sConfigMgr->GetInt32("DB", "AsyncThreads", 1));
			if (asyncThreads < 1 || asyncThreads > 32)
			{
				LOG_ERROR("sql.driver", "%s database: invalid number of worker threads specified. "
                "Please pick a value between 1 and 32.", name.c_str());
				return false;
			}

			uint8 const synchThreads = uint8(sConfigMgr->GetInt32("DB", "SynchThreads", 1));
			SetConnectionInfo(dbString, asyncThreads, synchThreads);

			if (uint32 error = Open())
			{
				if ((error == ER_BAD_DB_ERROR) && updatesEnabledForThis)
				{
					if (DBUpdater<T>::Create(pool) && (!Open()))
						error = 0;
				}

				if (error)
				{
					LOG_ERROR("sql.driver", "\n%s DatabasePool %s NOT opened. There were errors opening the MySQL connections. Check your SQLDriverLogFile ", name.c_str());
					return false;
				}
			}

			m_close.push([this]
			{
				Close();
			});
			return true;

		});

		if (updatesEnabledForThis)
		{
			m_populate.push([this, &pool, name]() -> bool
			{
				if (!DBUpdater<T>::Populate(pool))
				{
					LOG_ERROR("sql.driver", "Could not populate the %s database, see log for details.", name.c_str());
					return false;
				}
				return true;
			});

			m_update.push([this, &pool, name]() -> bool
			{
				if (!DBUpdater<T>::Update(pool))
				{
					LOG_ERROR("sql.driver", "Could not update the %s database, see log for details.", name.c_str());
					return false;
				}
				return true;
			});
		}

		m_prepare.push([this, name]() -> bool
		{
			if (!PrepareStatements())
			{
				LOG_ERROR("sql.driver", "Could not prepare statements of the %s database, see log for details.", name.c_str());
				return false;
			}
			return true;
		});

		return Load();
	}

	template<typename T>
	bool DBWorkerPool<T>::Load()
	{
		if (!m_updateFlags)
			LOG_INFO("sql.updates", "Automatic database updates are disabled for all databases!");

		if (!Process(m_open))
			return false;

		if (!Process(m_populate))
			return false;

		if (!Process(m_update))
			return false;

		if (!Process(m_prepare))
			return false;

		return true;
	}

	template<typename T>
	bool DBWorkerPool<T>::Process(std::queue<Predicate>& queue)
	{
		while (!queue.empty())
		{
			if (!queue.front()())
			{
				while (!m_close.empty())
				{
					m_close.top()();
					m_close.pop();
				}

				return false;
			}
			queue.pop();
		}
		return true;
	}

	template<typename T>
	PreparedStatement * DBWorkerPool<T>::GetPreparedStatement(PreparedStatementIndex index)
	{
		return new PreparedStatement(index, m_preparedStatementSize[index]);
	}

	template<typename T>
	QueryResult DBWorkerPool<T>::Query(char const * sql, T * connection)
	{
		if (!connection)
			connection = GetFreeConnection();

		ResultSet* result = connection->Query(sql);
		connection->Unlock();
		if (!result || !result->GetRowCount() || !result->NextRow())
		{
			delete result;
			return QueryResult(nullptr);
		}

		return QueryResult(result);
	}

	template<typename T>
	PreparedQueryResult DBWorkerPool<T>::Query(PreparedStatement * stmt)
	{
		auto connection = GetFreeConnection();
		PreparedResultSet* ret = connection->Query(stmt);
		connection->Unlock();

		//! Delete proxy-class. Not needed anymore
		delete stmt;

		if (!ret || !ret->GetRowCount())
		{
			delete ret;
			return PreparedQueryResult(nullptr);
		}

		return PreparedQueryResult(ret);
	}

	template<typename T>
	void DBWorkerPool<T>::DirectExecute(char const * sql)
	{
		if (sql == nullptr)
			return;

		T* connection = GetFreeConnection();
		connection->Execute(sql);
		connection->Unlock();
	}

	template<typename T>
	void DBWorkerPool<T>::DirectExecute(PreparedStatement * stmt)
	{
		T* connection = GetFreeConnection();
		connection->Execute(stmt);
		connection->Unlock();

		//! Delete proxy-class. Not needed anymore
		delete stmt;
	}

	template<typename T>
	QueryCallback DBWorkerPool<T>::AsyncQuery(PreparedStatement * stmt)
	{
		PreparedQueryResultFuture result;
		return QueryCallback(std::move(result));
	}

	template<typename T>
	uint32 DBWorkerPool<T>::OpenConnections(InternalIndex type, uint8 numConnections)
	{
		for (uint8 i = 0; i < numConnections; ++i)
		{
			auto connection = [&]
			{
				switch (type)
				{
				case IDX_ASYNC:
					return std::make_unique<T>(m_queue.get(), *m_connectionInfo);
				case IDX_SYNCH:
					return std::make_unique<T>(*m_connectionInfo);
				default:
					abort();
				}
			}();

			if (uint32 error = connection->Open())
			{
				m_connections[type].clear();
				return error;
			}
			else if (mysql_get_server_version(connection->GetHandle()) < MIN_MYSQL_SERVER_VERSION)
			{
				LOG_ERROR("sql.driver", "Lendy 不支持 MySQL 5.1 以下版本");
				return 1;
			}
			else
			{
				m_connections[type].push_back(std::move(connection));
			}
		}
		return 0;
	}

	template<typename T>
	T * DBWorkerPool<T>::GetFreeConnection()
	{
		uint8 i = 0;
		auto const connCounts = m_connections[IDX_SYNCH].size();
		T* connection = nullptr;

		for (;;)
		{
			connection = m_connections[IDX_SYNCH][++i % connCounts].get();
			if (connection->LockIfReady())
			{
				break;
			}
		}
		return connection;
	}

	template<typename T>
	char const * DBWorkerPool<T>::GetDatabaseName() const
	{
		return m_connectionInfo->database.c_str();
	}
	
	template class LENDY_COMMON_API DBWorkerPool<LogonDatabaseConnection>;
}