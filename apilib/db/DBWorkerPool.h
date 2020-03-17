#ifndef DB_ENGINE_H
#define DB_ENGINE_H

#include "Define.h"
#include "DBEnvHeader.h"
#include <functional>
#include <array>
#include <string>
#include <vector>
#include <queue>
#include <stack>

namespace Util
{
	template<typename T>
	class ProducerConsumerQueue;
}

namespace DB
{
	class QueryCallback;
	class SQLOperation;
	class MySQLConnection;
	struct MySQLConnectionInfo;

	template<typename T>
	class DBWorkerPool
	{
	public:
		DBWorkerPool();
		virtual ~DBWorkerPool();

	private:
		enum InternalIndex
		{
			IDX_ASYNC,
			IDX_SYNCH,
			IDX_SIZE
		};

		//启动接口
	public:
		bool Start(DBWorkerPool<T> &pool);

	private:
		using Predicate = std::function<bool()>;
		using Closer = std::function<void()>;

		bool Load();
		bool Process(std::queue<Predicate>& queue);

		uint32 const m_updateFlags;
		EXPORT_BEGIN
		std::queue<Predicate> m_open, m_populate, m_update, m_prepare;
		std::stack<Closer> m_close;
		EXPORT_END
		
		//////////////////////////////////////////////////////////////////
		//查询函数
	public:
		//获取预设
		typedef typename T::Statements PreparedStatementIndex;
		PreparedStatement* GetPreparedStatement(PreparedStatementIndex index);
		
		//同步查询
		QueryResult Query(char const* sql, T* connection = nullptr);

		//同步查询
		PreparedQueryResult Query(PreparedStatement* stmt);

		void DirectExecute(char const* sql);
		
		//异步查询
		QueryCallback AsyncQuery(PreparedStatement* stmt);

		inline MySQLConnectionInfo const* GetConnectionInfo() const
		{
			return m_connectionInfo.get();
		}

	private:
		//连接信息
		void SetConnectionInfo(std::string const& infoString, uint8 const asyncThreads, uint8 const synchThreads);

		//连接数据库
		uint32 Open();

		//清空数据
		void Close();

		//预加载指令
		bool PrepareStatements();

		//开启连接
		uint32 OpenConnections(InternalIndex type, uint8 numConnections);

		//空闲连接
		T* GetFreeConnection();

		//数据库名
		char const* GetDatabaseName() const;

		uint8	m_asyncThreads, m_synchThreads;
		EXPORT_BEGIN
		std::unique_ptr<Util::ProducerConsumerQueue<SQLOperation*>>			m_queue;
		std::array<std::vector<std::unique_ptr<T>>, IDX_SIZE>				m_connections;
		std::unique_ptr<MySQLConnectionInfo>								m_connectionInfo;
		std::vector<uint8>													m_preparedStatementSize;
		EXPORT_END
	};
}

#endif