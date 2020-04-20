#ifndef STRUCT_H
#define STRUCT_H

#include "Define.h"

#pragma pack(1)

namespace Comm
{
	//游戏种类
	struct tagGameKind
	{
		uint16							wSortID;							//排序索引
		uint16							wKindID;							//类型索引
		uint16							wGameID;							//模块索引

		uint32							dwOnLineCount;						//在线人数
		uint32							dwAndroidCount;						//机器人数
		uint32							dwFullCount;						//满员人数
	};

	//游戏房间
	struct tagGameRoom
	{
		uint16							wKindID;							//名称索引
		uint16							wSortID;							//排序索引
		uint16							wServerID;							//房间索引
		uint16                          wServerKind;                        //房间类型
		uint16							wServerLevel;						//房间等级
		uint16							wServerPort;						//房间端口
		uint16							wTableCount;						//桌子数
		uint64							lCellScore;							//单元积分
		uint64							lEnterScore;						//进入积分

		uint32							dwServerRule;						//房间规则

		uint32							dwOnLineCount;						//在线人数
		uint32							dwAndroidCount;						//机器人数
		uint32							dwFullCount;						//满员人数

		char							szServerAddr[32];					//房间名称
		char							szServerName[32];					//房间名称
	};


	//广场子项
	struct tagGameLogon
	{
		uint16							wPlazaID;							//广场标识
		char							szServerAddr[32];					//服务地址
		char							szServerName[32];					//服务器名
	};
}

#pragma pack()


#endif