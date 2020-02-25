#include "DBWorker.h"
#include "ProducerConsumerQueue.h"
#include "SQLOperation.h"
#include "DBWorker.h"

namespace DB
{
	DBWorker::DBWorker(ProducerConsumerQueue<SQLOperation*>* newQueue, MySQLConnection * connection):
		m_connection(connection),
		m_queue(newQueue),
		m_cancelationToken(false)
	{
		m_workerThread = std::thread(&DBWorker::WorkerThread, this);
	}

	DBWorker::~DBWorker()
	{
		m_cancelationToken = true;
		//m_queue->Cancel();
		m_workerThread.join();
	}

	void DBWorker::WorkerThread()
	{
		if (!m_queue)
		{
			return;
		}

		for (;;)
		{
			SQLOperation* operation = nullptr;

			m_queue->WaitAndPop(operation);

			if (m_cancelationToken || !operation)
			{
				return;
			}

			operation->SetConnection(m_connection);
			operation->call();

			delete operation;
		}
	}
}