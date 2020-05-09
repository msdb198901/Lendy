#ifndef HEADER_LOGON_H
#define HEADER_LOGON_H

#ifdef LENDY_COMPILER_14
#include <unordered_map>
#else
#include <map>
#endif
#include "DBEnvHeader.h"
#include "KernelEngineHead.h"

//命名空间
namespace Game
{
	using namespace Net;
	using namespace DB;
}

//常量定义
namespace Game
{
#define NETWORK_CORRESPOND			1									//登录连接

#define IDI_MAIN_MODULE_START		1									//起始标识
#define IDI_MAIN_MODULE_FINISH		99									//终止标识

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

	enum SMT
	{
		SMT_CHAT				= 0x0001,								//聊天消息
		SMT_EJECT				= 0x0002,								//弹出消息
		SMT_GLOBAL				= 0x0004,								//全局消息
		SMT_PROMPT				= 0x0008,								//提示消息
		SMT_TABLE_ROLL			= 0x0010,								//滚动消息

		SMT_CLOSE_ROOM			= 0x0100,								//关闭房间
		SMT_CLOSE_GAME			= 0x0200,								//关闭游戏
		SMT_CLOSE_LINK			= 0x0400,								//中断连接
		SMT_CLOSE_INSURE		= 0x0800								//关闭银行
	}; 

	enum GameTimerID
	{
		//桌子范围
		IDI_TABLE_MODULE_START  = 10000,								//起始标识
		IDI_TABLE_MODULE_FINISH = 50000,								//终止标识


		IDI_TABLE_SINK_RANGE	= 50,									//标识范围
		IDI_TABLE_MODULE_RANGE	= 100,									//标识范围
	};

	enum LogonErrorRoom
	{
		LER_NORMAL				= 0x00,									//常规离开
		LER_SYSTEM				= 0x01,									//系统原因
		LER_NETWORK				= 0x02,									//网络原因
		LER_USER_IMPACT			= 0x03,									//用户冲突
		LER_SERVER_FULL			= 0x04,									//人满为患
		LER_SERVER_CONDITIONS	= 0x05									//条件限制
	};

	enum LogonErrorCode
	{
		LEC_NONE = 0,

		LEC_LIMIT_IP = 1,
		LEC_LIMIT_MAC = 2,
		LEC_LIMIT_FREEZE = 4,

		LEC_ROOM_FULL = 5,
		LEC_ROOM_ENTER_SCORE_LESS = 7,

		LEC_USER_PLAYING = 100,
		LEC_USER_ROOM_FULL = 101,
		LEC_USER_TABLE_NOT_CHAIR = 102,
		LEC_USER_ENTER_TABLE = 103,

		LEC_PW_EMPTY = 200,
		LEC_MAX_CODE
	};

	typedef std::pair<LogonErrorCode, std::string> K_LE;
#ifdef LENDY_COMPILER_14
	typedef std::unordered_map<LogonErrorCode, std::string> LogonErrorContainer;
#else
	typedef std::map<LogonErrorCode, std::string> LogonErrorContainer;
#endif
	static LogonErrorContainer LogonError =
	{
		K_LE(LEC_LIMIT_IP, "抱歉地通知您，系统禁止了您所在的 IP 地址的登录功能，请联系客户服务中心了解详细情况！"),
		K_LE(LEC_LIMIT_MAC, "抱歉地通知您，系统禁止了您的机器的登录功能，请联系客户服务中心了解详细情况！"),
		K_LE(LEC_LIMIT_FREEZE, "您的帐号暂时处于冻结状态，请联系客户服务中心了解详细情况！"),

		K_LE(LEC_ROOM_FULL, "抱歉，由于此房间已经人满，不能继续进入了！"),
		K_LE(LEC_ROOM_ENTER_SCORE_LESS, "进入该房间需要%ld金币！"),

		K_LE(LEC_USER_PLAYING, "您正在游戏中，暂时不能离开，请先结束当前游戏！"),
		K_LE(LEC_USER_ROOM_FULL, "当前游戏房间已经人满为患了，暂时没有可以让您加入的位置，请稍后再试！"),
		K_LE(LEC_USER_TABLE_NOT_CHAIR, "由于此游戏桌暂时没有可以让您加入的位置了，请选择另外的游戏桌！"),
		K_LE(LEC_USER_ENTER_TABLE, "欢迎您进入“%s”游戏，祝您游戏愉快！"),

		K_LE(LEC_PW_EMPTY, "很抱歉，您的登录密码错误，不允许继续进入！"),
	};

	//控制结果
	struct ControlResult
	{
		uint8							cbSuccess;							//成功标志
	};

	struct tagGameAddressOption
	{
		char							szIP[32];
		uint16							wPort;
		uint16							wKindID;
		uint16							wThreadCount;
	};
}


#endif