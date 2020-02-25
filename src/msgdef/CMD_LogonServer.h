#ifndef CMD_LONGON_HEAD_FILE
#define CMD_LONGON_HEAD_FILE

#include "Define.h"

#pragma pack(1)

using namespace comm;

//登录命令
#define MDM_MB_LOGON				100									//广场登录
#define SUB_MB_LOGON_VISITOR		5									//游客登录

//游客登录
struct CMD_MB_LogonVisitor
{
	//系统信息
	uint16							wModuleID;							//模块标识
	uint64							dwPlazaVersion;						//广场版本
	char							szAgentID[LEN_ACCOUNTS];			//代理标识
	uint8                           cbDeviceType;                       //设备类型

	//连接信息
	char							szMachineID[LEN_MACHINE_ID];		//机器标识
	char							szMobilePhone[LEN_MOBILE_PHONE];	//电话号码
};

#pragma pack()

#endif