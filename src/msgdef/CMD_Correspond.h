#ifndef CMD_CORRESPOND_HEAD_FILE
#define CMD_CORRESPOND_HEAD_FILE


#include "Define.h"

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
#define SUB_CS_S_ROOM_FINISH		115									//房间完成

//房间人数
struct CMD_CS_S_ServerOnLine
{
	uint16							wServerID;							//房间标识
	uint32							dwOnLineCount;						//在线人数
	uint32							dwAndroidCount;						//机器人数
};

#pragma pack()

#endif