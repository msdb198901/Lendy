#ifndef PRODUCER_CONSUMER_QUEUE_H
#define PRODUCER_CONSUMER_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <atomic>
#include <type_traits>

namespace Util
{
	template <typename T>
	class ProducerConsumerQueue
	{
	public:
		ProducerConsumerQueue<T>() : m_shutdown(false) {}

		void Push(const T& value)
		{
			std::lock_guard<std::mutext> _lock(m_queueLock);
			m_queue.push(std::move(value));
			m_condition.notify_one();
		}

		bool Empty()
		{
			std::lock_guard<std::mutex> _lock(m_queueLock);
			return m_queue.empty();
		}

		bool Pop(T& value)
		{
			std::lock_guard<std::mutex> _lock(m_queueLock);
			if (m_queue.empty() || m_shutdown)
			{
				return false;
			}

			value = m_queue.front();
			m_queue.pop();
			return true;
		}

		void WaitAndPop(T& value)
		{
			std::unique_lock<std::mutex> _lock(m_queueLock);
			while (m_queue.empty() && !m_shutdown)
			{
				m_condition.wait(_lock);
			}

			if (m_queue.empty() || m_shutdown)
			{
				return ;
			}

			value = m_queue.front();
			m_queue.pop();
		}

		void Cancel()
		{
			std::unique_lock<std::mutex> _lock(m_queueLock);

			while (!m_queue.empty())
			{
				T &value = m_queue.front();
				DeleteQueuedObject(value);
				m_queue.pop();
			}

			m_shutdown = true;
			m_condition.notify_all();
		}

	private:
		template<typename E = T>
		typename std::enable_if<std::is_pointer<E>::value>::type DeleteQueuedObject(E& obj) { delete obj; }

		template<typename E = T>
		typename std::enable_if<!std::is_pointer<E>::value>::type DeleteQueuedObject(E const& /*packet*/) { }
	private:
		std::mutex				m_queueLock;
		std::queue<T>			m_queue;
		std::condition_variable m_condition;
		std::atomic<bool>		m_shutdown;
	};
}

#endif