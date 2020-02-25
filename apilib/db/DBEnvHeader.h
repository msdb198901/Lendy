#ifndef DB_ENV_HEADER_H
#define DB_ENV_HEADER_H

#include "Define.h"
#include <future>
#include <memory>

// mysql
typedef struct st_mysql MYSQL;
typedef struct st_mysql_res MYSQL_RES;
typedef struct st_mysql_field MYSQL_FIELD;
typedef struct st_mysql_bind MYSQL_BIND;
typedef struct st_mysql_stmt MYSQL_STMT;

namespace DB
{
	class Field;
	class ResultSet;
	class QueryCallback;

	typedef std::shared_ptr<ResultSet> QueryResult;
	typedef std::future<QueryResult> QueryResultFuture;
	typedef std::promise<QueryResult> QueryResultPromise;

	class PreparedStatement;
	class PreparedResultSet;
	typedef std::shared_ptr<PreparedResultSet> PreparedQueryResult;
	typedef std::future<PreparedQueryResult> PreparedQueryResultFuture;
	typedef std::promise<PreparedQueryResult> PreparedQueryResultPromise;

	class Transaction;
	typedef std::shared_ptr<Transaction> SQLTransaction;
}

#endif