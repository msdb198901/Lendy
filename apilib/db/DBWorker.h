#ifndef DB_WORKER_H
#define DB_WORKER_H

#include "Define.h"
#include <atomic>
#include <thread>

namespace Util
{
	template <typename T>
	class ProducerConsumerQueue;
}

namespace DB
{
	using Util::ProducerConsumerQueue;

	class SQLOperation;
	class MySQLConnection;

	class DBWorker
	{
	public:
		DBWorker(ProducerConsumerQueue<SQLOperation*> *newQueue, MySQLConnection* connection);

		virtual ~DBWorker();

	private:

		void WorkerThread();

		std::thread								m_workerThread;
		std::atomic<bool>						m_cancelationToken;
		ProducerConsumerQueue<SQLOperation*>*	m_queue;
		MySQLConnection*						m_connection;
		
		DELETE_COPY_ASSIGN(DBWorker);
	};
}

#endif