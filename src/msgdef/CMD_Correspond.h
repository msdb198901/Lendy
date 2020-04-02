#ifndef CMD_CORRESPOND_HEAD_FILE
#define CMD_CORRESPOND_HEAD_FILE


#include "Define.h"

#pragma pack(1)

#define MDM_CS_REGISTER				1									//服务注册

//服务注册
#define SUB_CS_C_REGISTER_PLAZA		100									//注册广场

//注册结果
#define SUB_CS_S_REGISTER_FAILURE	200									//注册失败

//注册广场
struct CMD_CS_C_RegisterPlaza
{
	wchar_t							szServerAddr[32];					//服务地址
	wchar_t							szServerName[32];					//服务器名
};

//注册失败
struct CMD_CS_S_RegisterFailure
{
	long							lErrorCode;							//错误代码
	wchar_t							szDescribeString[128];				//错误消息
};

//////////////////////////////////////////////////////////////////////////////////
//服务信息

#define MDM_CS_SERVICE_INFO			2									//服务信息
//房间命令
#define SUB_CS_S_SERVER_INFO		110									//房间信息
#define SUB_CS_S_SERVER_ONLINE		111									//房间人数
#define SUB_CS_S_SERVER_INSERT		112									//房间列表
#define SUB_CS_S_SERVER_FINISH		115									//房间完成

//房间人数
struct CMD_CS_S_ServerOnLine
{
	uint16							wServerID;							//房间标识
	uint32							dwOnLineCount;						//在线人数
	uint32							dwAndroidCount;						//机器人数
};

#pragma pack()

#endif