#ifndef CMD_LONGON_HEAD_FILE
#define CMD_LONGON_HEAD_FILE

#include "Define.h"

#pragma pack(1)

using namespace Comm;

//登录命令
#define MDM_MB_LOGON				100									//广场登录
#define SUB_MB_LOGON_VISITOR		5									//游客登录


//登录结果
#define SUB_MB_LOGON_SUCCESS		100									//登录成功
#define SUB_MB_LOGON_FAILURE		101									//登录失败

//游客登录
struct CMD_MB_LogonVisitor
{
	//系统信息
	uint16							wModuleID;							//模块标识
	uint32							dwPlazaVersion;						//广场版本
	wchar							szAgentID[LEN_ACCOUNTS];			//代理标识
	uint8                           cbDeviceType;                       //设备类型

	//连接信息
	wchar							szMachineID[LEN_MACHINE_ID];		//机器标识
	wchar							szMobilePhone[LEN_MOBILE_PHONE];	//电话号码
};

//登录成功
struct CMD_MB_LogonSuccess
{
	uint16							wFaceID;							//头像标识
	uint8							cbGender;							//用户性别
	uint32							dwCustomID;							//自定头像
	uint32							dwUserID;							//用户 I D
	uint32							dwGameID;							//游戏 I D
	uint32							dwSpreaderID;						//推广ID
	uint32							dwExperience;						//经验数值
	wchar							szAccounts[LEN_ACCOUNTS];			//用户帐号
	wchar							szNickName[LEN_NICKNAME];			//用户昵称
	wchar							szAliPayAcccount[30];				//支付宝账户
	wchar							szBinkID[20];						//银行卡账户
	wchar							szDynamicPass[LEN_PASSWORD];		//动态密码

	//财富信息
	SCORE							lUserScore;							//用户游戏币
	SCORE							lUserInsure;						//用户银行	

	//扩展信息
	uint8							cbInsureEnabled;					//使能标识
	uint8							cbIsAgent;							//代理标识
	uint8							cbMoorMachine;						//锁定机器

	//约战房相关
	int								TodayAlmsCount;						//每日低保已领取次数
	uint32							dwLockServerID;						//锁定房间
	uint32							dwKindID;							//游戏类型

	wchar							szMobilePhone[LEN_MOBILE_PHONE];	//绑定手机
};

//登录失败
struct CMD_MB_LogonFailure
{
	uint32							lResultCode;						//错误代码
	wchar							szDescribe[LEN_ERROR_DESCRIBE];		//描述消息
};


//房间列表
#define MDM_MB_SERVER_LIST			101									//列表信息

#define SUB_MB_KIND_LIST			100									//种类列表
#define SUB_MB_ROOM_LIST			101									//房间列表
#define SUB_MB_LIST_FINISH			200									//列表完成


//游戏种类
struct CMD_MB_GameKindItem
{
	uint16							wTypeID;							//类型索引
	uint16							wJoinID;							//挂接索引
	uint16							wSortID;							//排序索引
	uint16							wKindID;							//类型索引
	uint16							wGameID;							//模块索引
	uint16							wRecommend;							//推荐游戏
	uint16							wGameFlag;							//游戏标志
	uint32							dwOnLineCount;						//在线人数
	uint32							dwAndroidCount;						//机器人数
	uint32							dwDummyCount;						//虚拟人数
	uint32							dwFullCount;						//满员人数
	uint32							dwSuportType;						//支持类型
	wchar							szKindName[32];						//游戏名字
	wchar							szProcessName[32];					//进程名字
};

//游戏房间
struct CMD_MB_GameRoomItem
{
	uint16							wKindID;							//名称索引
	uint16							wNodeID;							//节点索引
	uint16							wSortID;							//排序索引
	uint16							wServerID;							//房间索引
	uint16                          wServerKind;                        //房间类型
	uint16							wServerType;						//房间类型
	uint16							wServerLevel;						//房间等级
	uint16							wServerPort;						//房间端口
	SCORE							lCellScore;							//单元积分
	uint8							cbEnterMember;						//进入会员
	SCORE							lEnterScore;						//进入积分
	SCORE							lTableScore;						//坐下游戏积分
	uint32							dwServerRule;						//房间规则
	uint32							dwOnLineCount;						//在线人数
	uint32							dwAndroidCount;						//机器人数
	uint32							dwFullCount;						//满员人数
	wchar							szServerAddr[32];					//房间名称
	wchar							szServerName[32];					//房间名称
	
	//私人房添加
	uint32							dwSurportType;						//支持类型
	uint16							wTableCount;						//桌子数目
	uint32							dwDummyCount;						//虚拟人数
};

#pragma pack()

#endif