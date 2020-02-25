#ifndef DEFINE_H
#define DEFINE_H

#include <cstddef>
#include <cinttypes>
#include "Macro.h"

#ifdef _DEBUG
#include <assert.h>
#endif

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef volatile int vint;
typedef unsigned char BYTE;
typedef unsigned long ulong;
typedef uint64 uuid64;


namespace comm
{
    static const uint32 MaxModuleIDCount = 8;

	static const uint32 MaxMsgLen = 1024 * 1024;   

	static const int MaxFullLen = 2048;            // 支持的最大全路径文件名长度

	static const int ConfigFileLen = 128;	

	static const int send_buff_block_size = 1024;

	static const int recv_buff_block_size = 1024 << 4;


	static const int PATH_MAX = 256;
	
	////////////////////////网狐框架常用变量
	//帐号长度
	static const int LEN_ACCOUNTS		= 32;
	//序列长度
	static const int LEN_MACHINE_ID		= 33;
	//移动电话
	static const int LEN_MOBILE_PHONE	= 12;	


}

#endif