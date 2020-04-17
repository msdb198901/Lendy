#include "QueryCallback.h"
#include <assert.h>

namespace DB
{
	template<typename T, typename... Args>
	inline void Construct(T& t, Args&&... args)
	{
		new (&t) T(std::forward<Args>(args)...);
	}

	template<typename T>
	inline void Destroy(T& t)
	{
		t.~T();
	}

	template<typename T>
	void ConstructActiveMember(T * obj)
	{
		if (!obj->m_bPrepared)
		{
			Construct(obj->m_string);
		}
		else
		{
			Construct(obj->m_prepared);
		}
	}

	template<typename T>
	void DestroyActiveMember(T * obj)
	{
		if (!obj->m_bPrepared)
		{
			Destroy(obj->m_string);
		}
		else
		{
			Destroy(obj->m_prepared);
		}
	}

	template<typename T>
	void MoveFrom(T * to, T && from)
	{
		assert(to->m_bPrepared == from.m_bPrepared);

		if (!to->m_bPrepared)
		{
			to->m_string = std::move(from.m_string);
		}
		else
		{
			to->m_prepared = std::move(from.m_prepared);
		}
	}

	struct QueryCallback::QueryCallbackData
	{
	public:
		friend class QueryCallback;

		QueryCallbackData(std::function<void(QueryCallback&, QueryResult)>&& callback) :
			m_string(std::move(callback)), 
			m_bPrepared(false) 
		{
		}

		QueryCallbackData(std::function<void(QueryCallback&, PreparedQueryResult)>&& callback) : 
			m_prepared(std::move(callback)), 
			m_bPrepared(true)
		{
		}

		QueryCallbackData(QueryCallbackData&& right)
		{
			m_bPrepared = right.m_bPrepared;
			ConstructActiveMember(this);
			MoveFrom(this, std::move(right));
		}

		QueryCallbackData& operator=(QueryCallbackData&& right)
		{
			if (this != &right)
			{
				if (this->m_bPrepared != right.m_bPrepared)
				{
					DestroyActiveMember(this);
					m_bPrepared = right.m_bPrepared;
					ConstructActiveMember(this);
				}
				MoveFrom(this, std::move(right));
			}
			return *this;
		}

		virtual ~QueryCallbackData() { DestroyActiveMember(this); }

	private:
		DELETE_COPY_ASSIGN(QueryCallbackData);

		template<typename T> friend void ConstructActiveMember(T* obj);
		template<typename T> friend void DestroyActiveMember(T* obj);
		template<typename T> friend void MoveFrom(T* to, T&& from);

		union
		{
			std::function<void(QueryCallback&, QueryResult)> m_string;
			std::function<void(QueryCallback&, PreparedQueryResult)> m_prepared;
		};
		bool m_bPrepared;
	};

	QueryCallback::QueryCallback(QueryResultFuture && result)
	{
		m_bPrepared = false;
		Construct(m_string, std::move(result));
	}

	QueryCallback::QueryCallback(PreparedQueryResultFuture && result)
	{
		m_bPrepared = true;
		Construct(m_prepared, std::move(result));
	}

	QueryCallback::QueryCallback(QueryCallback && right)
	{
		m_bPrepared = right.m_bPrepared;
		ConstructActiveMember(this);
		MoveFrom(this, std::move(right));
		m_callbacks = std::move(right.m_callbacks);
	}

	QueryCallback& QueryCallback::operator=(QueryCallback && right)
	{
		if (this != &right)
		{
			if (this->m_bPrepared != right.m_bPrepared)
			{
				DestroyActiveMember(this);
				m_bPrepared = right.m_bPrepared;
				ConstructActiveMember(this);
			}
			MoveFrom(this, std::move(right));
			m_callbacks = std::move(right.m_callbacks);
		}
		return *this;
	}

	QueryCallback::~QueryCallback()
	{
		DestroyActiveMember(this);
	}

	QueryCallback && QueryCallback::WithCallback(std::function<void(QueryResult)>&& callback)
	{
		return WithChainingCallback([callback](QueryCallback& /*this*/, QueryResult result) { callback(std::move(result)); });
	}

	QueryCallback && QueryCallback::WithPreparedCallback(std::function<void(PreparedQueryResult)>&& callback)
	{
		return WithChainingPreparedCallback([callback](QueryCallback& /*this*/, PreparedQueryResult result) { callback(std::move(result)); });
	}

	QueryCallback && QueryCallback::WithChainingCallback(std::function<void(QueryCallback&, QueryResult)>&& callback)
	{
		assert(!m_callbacks.empty() || !m_bPrepared);
		m_callbacks.emplace(std::move(callback));
		return std::move(*this);	
	}

	QueryCallback && QueryCallback::WithChainingPreparedCallback(std::function<void(QueryCallback&, PreparedQueryResult)>&& callback)
	{
		assert(!m_callbacks.empty() || m_bPrepared);
		m_callbacks.emplace(std::move(callback));
		return std::move(*this);
	}

	void QueryCallback::SetNextQuery(QueryCallback && next)
	{
		MoveFrom(this, std::move(next));
	}

	QueryCallback::Status QueryCallback::InvokeIfReady()
	{
		QueryCallbackData& callback = m_callbacks.front();
		auto checkStateAndReturnCompletion = [this]()
		{
			m_callbacks.pop();
			bool bNext = !m_bPrepared ? m_string.valid() : m_prepared.valid();
			if (m_callbacks.empty())
			{
				assert(!bNext);
				return Completed;
			}
		
			if (!bNext)
			{
				return Completed;
			}

			assert(m_bPrepared == m_callbacks.front().m_bPrepared);
			return NextStep;
		};

		if (!m_bPrepared)
		{
			if (m_string.valid() && m_string.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			{
				QueryResultFuture f(std::move(m_string));
				std::function<void(QueryCallback&, QueryResult)> cb(std::move(callback.m_string));
				cb(*this, f.get());
				return checkStateAndReturnCompletion();
			}
		}
		else
		{
			if (m_prepared.valid() && m_prepared.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			{
				PreparedQueryResultFuture f(std::move(m_prepared));
				std::function<void(QueryCallback&, PreparedQueryResult)> cb(std::move(callback.m_prepared));
				cb(*this, f.get());
				return checkStateAndReturnCompletion();
			}
		}

		return NotReady;
	}
}