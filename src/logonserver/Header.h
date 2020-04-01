#ifndef HEADER_LOGON_H
#define HEADER_LOGON_H

#include <unordered_map>

namespace Logon
{
	using namespace Net;
	using namespace DB;
}

namespace Logon
{
#define NETWORK_CORRESPOND			1									//登录连接

	enum ServiceUnitsControl
	{
		SUC_CONNECT_CORRESPOND  = 1,
		SUC_LOAD_DB_GAME_LIST	= 2
	};

	enum UIDataControl
	{
		UDC_CORRESPOND_RESULT	= 1,
		UDC_LOAD_DB_LIST_RESULT = 2
	};

	enum LogonErrorCode
	{
		LEC_NONE = 0,
		LEC_LIMIT_IP = 1,
		LEC_LIMIT_MAC = 2,
		LEC_LIMIT_FREEZE = 4,
		LEC_MAX_CODE
	};

	typedef std::pair<LogonErrorCode, std::string> K;
	typedef std::unordered_map<LogonErrorCode, std::string> LogonErrorContainer;
	static LogonErrorContainer LogonError =
	{
		K(LEC_LIMIT_IP, "抱歉地通知您，系统禁止了您所在的 IP 地址的登录功能，请联系客户服务中心了解详细情况！"),
		K(LEC_LIMIT_MAC, "抱歉地通知您，系统禁止了您的机器的登录功能，请联系客户服务中心了解详细情况！"),
		K(LEC_LIMIT_FREEZE, "您的帐号暂时处于冻结状态，请联系客户服务中心了解详细情况！")
	};

	//控制结果
	struct ControlResult
	{
		uint8							cbSuccess;							//成功标志
	};
}

#endif