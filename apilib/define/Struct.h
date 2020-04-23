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

	//用户信息
	struct tagUserInfo
	{
		//基本属性
		uint32							dwUserID;							//用户 I D
		uint32							dwGameID;							//游戏 I D
		char							szNickName[LEN_NICKNAME];			//用户昵称
		char							szMobilePhone[LEN_MOBILE_PHONE];	//手机号码

		//头像信息
		uint16							wFaceID;							//头像索引

		//用户资料
		uint8							cbGender;							//用户性别

		//用户状态
		uint16							wTableID;							//桌子索引
		uint16							wLastTableID;					    //游戏桌子
		uint16							wChairID;							//椅子索引
		uint8							cbUserStatus;						//用户状态

		//积分信息
		uint64							lScore;								//用户分数

		//游戏信息
		uint32							dwWinCount;							//胜利盘数
		uint32							dwLostCount;						//失败盘数
		uint32							dwDrawCount;						//和局盘数
		uint32							dwFleeCount;						//逃跑盘数
	};

	//附加属性
	struct tagUserInfoPlus
	{
		//登录信息
		uint32							dwLogonTime;						//登录时间
		uint32							dwInoutIndex;						//进出标识

		//连接信息
		uint16							wBindIndex;							//绑定索引
		uint32							dwClientAddr;						//连接地址
		char							szMachineID[LEN_MACHINE_ID];		//机器标识

		//附加变量
		uint32							dwUserRight;						//用户权限
		uint64							lLimitScore;						//限制积分

		//辅助变量
		bool							bAndroidUser;						//机器用户
		char							szPassword[LEN_MD5];				//桌子密码
	};
}

#pragma pack()


#endif