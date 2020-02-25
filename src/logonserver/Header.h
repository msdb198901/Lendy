#ifndef HEADER_LOGON_H
#define HEADER_LOGON_H

namespace Logon
{
	using namespace Net;
	using namespace DB;
}

namespace Logon
{
	enum ServiceUnitsControl
	{
		SUC_CONNECT_CORRESPOND  = 1,
		SUC_LOAD_DB_GAME_LIST	= 2
	};
}

#endif