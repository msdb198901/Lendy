#ifndef CMD_CORRESPOND_HEAD_FILE
#define CMD_CORRESPOND_HEAD_FILE


#include "Define.h"

#pragma pack(1)

#define MDM_CS_REGISTER				1									//服务注册

//服务注册
#define SUB_CS_C_REGISTER_PLAZA		100									//注册广场

//注册广场
struct CMD_CS_C_RegisterPlaza
{
	wchar_t							szServerAddr[32];					//服务地址
	wchar_t							szServerName[32];					//服务器名
};

#pragma pack()

#endif