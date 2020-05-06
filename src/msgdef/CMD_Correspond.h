#ifndef CMD_CORRESPOND_HEAD_FILE
#define CMD_CORRESPOND_HEAD_FILE


#include "Define.h"
#include "Struct.h"

#pragma pack(1)

#define MDM_CS_REGISTER				1									//服务注册

//服务注册
#define SUB_CS_C_REGISTER_LOGON		100									//注册广场
#define SUB_CS_C_REGISTER_ROOM		101									//注册房间

//注册结果
#define SUB_CS_S_REGISTER_FAILURE	200									//注册失败

//注册广场
struct CMD_CS_C_RegisterLogon
{
	char							szServerAddr[32];					//服务地址
	char							szServerName[32];					//服务器名
};

//注册失败
struct CMD_CS_S_RegisterFailure
{
	long							lErrorCode;							//错误代码
	char							szDescribeString[128];				//错误消息
};

//注册游戏
struct CMD_CS_C_RegisterRoom
{
	uint16							wKindID;							//名称索引
	uint16							wServerID;							//房间索引
	uint16							wServerPort;						//房间端口
	uint64							lCellScore;							//单元积分
	uint64							lEnterScore;						//进入积分
	uint32							dwOnLineCount;						//在线人数
	uint32							dwFullCount;						//满员人数
	uint32							wTableCount;						//桌子数目
	uint32							dwServerRule;						//房间规则
	char							szServerAddr[32];					//服务地址
	char							szServerName[32];					//房间名称
};

//////////////////////////////////////////////////////////////////////////////////
//服务信息

#define MDM_CS_ROOM_INFO			2									//服务信息
//房间命令
#define SUB_CS_S_ROOM_INFO			110									//房间信息
#define SUB_CS_S_ROOM_ONLINE		111									//房间人数
#define SUB_CS_S_ROOM_INSERT		112									//房间列表
#define SUB_CS_S_ROOM_REMOVE		114									//房间删除
#define SUB_CS_S_ROOM_FINISH		115									//房间完成

//房间人数
struct CMD_CS_S_RoomOnLine
{
	uint16							wServerID;							//房间标识
	uint32							dwOnLineCount;						//在线人数
	uint32							dwAndroidCount;						//机器人数
};

//房间删除
struct CMD_CS_S_RoomRemove
{
	uint16							wServerID;							//房间标识
};

//用户汇总
#define MDM_CS_USER_COLLECT			3									//用户汇总

//用户状态
#define SUB_CS_C_USER_ENTER			1									//用户进入
#define SUB_CS_C_USER_LEAVE			2									//用户离开
#define SUB_CS_C_USER_STATUS		4									//用户状态
//用户状态
#define SUB_CS_S_COLLECT_REQUEST	100									//汇总请求


//用户进入
struct CMD_CS_C_UserEnter
{
	//用户信息
	uint32							dwUserID;							//用户标识
	uint32							dwGameID;							//游戏标识
	char							szNickName[Comm::LEN_NICKNAME];		//用户昵称

	//详细信息
	Comm::tagUserInfo				userInfo;							//用户信息
};


//用户离开
struct CMD_CS_C_UserLeave
{
	uint32							dwUserID;							//用户标识
};


//用户状态
struct CMD_CS_C_UserStatus
{
	//用户信息
	uint32							dwUserID;							//用户标识
	uint8							cbUserStatus;						//用户状态
	uint16							wKindID;							//游戏标识
	uint16							wServerID;							//房间标识
	uint16							wTableID;							//桌子索引
	uint16							wChairID;							//椅子位置
};


#pragma pack()

#endif