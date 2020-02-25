#ifndef DB_EXPORTS_H
#define DB_EXPORTS_H

#include "DBWorkerPool.h"
#include "Implementation/LogonDatabase.h"

#include "Field.h"
#include "PreparedStatement.h"
#include "QueryCallback.h"
#include "QueryResult.h"

namespace DB
{
	LENDY_COMMON_API extern DBWorkerPool<LogonDatabaseConnection> LogonDatabasePool;
}

#endif