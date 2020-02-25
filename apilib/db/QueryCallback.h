#ifndef QUERY_CALLBACK_H
#define QUERY_CALLBACK_H

#include "Define.h"
#include "DBEnvHeader.h"
#include <functional>
#include <future>
#include <list>
#include <queue>
#include <utility>

namespace DB
{
	class LENDY_COMMON_API QueryCallback
	{
	public:
		enum Status
		{
			NotReady,
			NextStep,
			Completed
		};

		union
		{
			QueryResultFuture m_string;
			PreparedQueryResultFuture m_prepared;
		};

	public:
		explicit QueryCallback(QueryResultFuture&& result);
		explicit QueryCallback(PreparedQueryResultFuture&& result);
		QueryCallback(QueryCallback&& right);
		QueryCallback& operator=(QueryCallback&& right);
		
		DELETE_COPY_ASSIGN(QueryCallback);

		virtual ~QueryCallback();

		QueryCallback&& WithCallback(std::function<void(QueryResult)>&& callback);
		QueryCallback&& WithPreparedCallback(std::function<void(PreparedQueryResult)>&& callback);
	
		QueryCallback&& WithChainingCallback(std::function<void(QueryCallback&, QueryResult)>&& callback);
		QueryCallback&& WithChainingPreparedCallback(std::function<void(QueryCallback&, PreparedQueryResult)>&& callback);

		void SetNextQuery(QueryCallback&& next);

		Status InvokeIfReady();

	private:

		template<typename T> friend void ConstructActiveMember(T* obj);
		template<typename T> friend void DestroyActiveMember(T* obj);
		template<typename T> friend void MoveFrom(T* to, T&& from);

		bool m_bPrepared;

		struct QueryCallbackData;

		EXPORT_BEGIN
		std::queue<QueryCallbackData, std::list<QueryCallbackData>> m_callbacks;
		EXPORT_END
		
	};
}

#endif