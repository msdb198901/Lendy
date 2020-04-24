#ifndef CMD_GAME_HEAD_FILE
#define CMD_GAME_HEAD_FILE

#include "Define.h"

#pragma pack(1)

using namespace Comm;


//开始模式
enum StartGameMode
{
	START_MODE_ALL_READY			= 0x00,								//所有准备
	START_MODE_FULL_READY			= 0x01,								//满人开始
	START_MODE_PAIR_READY			= 0x02,								//配对开始
	START_MODE_TIME_CONTROL			= 0x10,								//时间控制
	START_MODE_MASTER_CONTROL		= 0x11								//管理控制
};

//结束原因
enum GameEndResult
{
	GER_NORMAL						= 0x00,								//常规结束
	GER_DISMISS						= 0x01,								//游戏解散
	GER_USER_LEAVE					= 0x02,								//用户离开
	GER_NETWORK_ERROR				= 0x03,								//网络错误
};

//游戏状态
enum GameSceneStatus
{
	GAME_STATUS_FREE				= 0	,								//空闲状态
	GAME_STATUS_PLAY				= 100,								//游戏状态
	GAME_STATUS_WAIT				= 200,								//等待状态
};

#define MDM_CM_SYSTEM				1000								//系统命令

#define SUB_CM_SYSTEM_MESSAGE		1									//系统消息
#define SUB_CM_ACTION_MESSAGE		2									//动作消息
#define SUB_CM_DOWN_LOAD_MODULE		3									//下载消息

//系统消息
struct CMD_CM_SystemMessage
{
	uint16							wType;								//消息类型
	uint16							wLength;							//消息长度
	wchar_t							szString[1024];						//消息内容
};

//////////////////////////////////////////////////////////////////////////////////
//登录命令

#define MDM_GR_LOGON				1									//登录信息

//登录模式
#define SUB_GR_LOGON_USERID			1									//I D 登录
#define SUB_GR_LOGON_MOBILE			2									//手机登录

//登录结果
#define SUB_GR_LOGON_SUCCESS		100									//登录成功
#define SUB_GR_LOGON_FAILURE		101									//登录失败
#define SUB_GR_LOGON_FINISH			102									//登录完成

//手机登录
struct CMD_GR_LogonMobile
{
	//版本信息
	uint16							wGameID;							//游戏标识
	uint32							dwProcessVersion;					//进程版本

	//桌子区域
	uint8                           cbDeviceType;                       //设备类型
	uint16                          wBehaviorFlags;                     //行为标识
	uint16                          wPageTableCount;                    //分页桌数

	//登录信息
	uint32							dwUserID;							//用户 I D
	wchar_t							szPassword[LEN_MD5];				//登录密码
	wchar_t                         szServerPasswd[LEN_PASSWORD];       //房间密码
	wchar_t							szMachineID[LEN_MACHINE_ID];		//机器标识
};

//登录成功
struct CMD_GR_LogonSuccess
{
	uint32							dwUserRight;						//用户权限
	uint32							dwMasterRight;						//管理权限
};

//登录失败
struct CMD_GR_LogonFailure
{
	uint32							lResultCode;						//错误代码
	wchar_t							szDescribeString[128];				//错误消息

	uint32							dwLockKindID;						//锁住房间的游戏KindID
	uint32							dwLockServerID;						//锁住房间的房间ServerID
};


//////////////////////////////////////////////////////////////////////////////////
//配置命令

#define MDM_GR_CONFIG				2									//配置信息

#define SUB_GR_CONFIG_SERVER		101									//房间配置
#define SUB_GR_CONFIG_FINISH		103									//配置完成

//////////////////////////////////////////////////////////////////////////////////
//房间配置
struct CMD_GR_ConfigServer
{
	//房间属性
	uint16							wTableCount;						//桌子数目
	uint16							wChairCount;						//椅子数目

	//房间配置
	uint16							wServerType;						//房间类型
	uint32							dwServerRule;						//房间规则
};

//////////////////////////////////////////////////////////////////////////////////
//用户命令

#define MDM_GR_USER					3									//用户信息

#define SUB_GR_USER_SITDOWN			3									//坐下请求
#define SUB_GR_USER_STANDUP			4									//起立请求

//用户状态
#define SUB_GR_USER_ENTER			100									//用户进入
#define SUB_GR_USER_STATUS			102									//用户状态
#define SUB_GR_USER_REQUEST_FAILURE	103									//请求失败

//用户状态
struct tagUserStatus
{
	uint16							wTableID;							//桌子索引
	uint16							wChairID;							//椅子位置
	uint8							cbUserStatus;						//用户状态
};

//用户信息
struct CMD_GR_UserInfoHead
{
	//用户属性
	uint32							dwGameID;							//游戏 I D
	uint32							dwUserID;							//用户 I D
	uint32							dwGroupID;							//社团 I D

	//头像信息
	uint16							wFaceID;							//头像索引
	uint32							dwCustomID;							//自定标识

	wchar_t							szNickName[LEN_NICKNAME];			//玩家昵称

	//用户属性
	bool							bIsAndroid;							//机器标识
	uint8							cbGender;							//用户性别
	uint8							cbMemberOrder;						//会员等级
	uint8							cbMasterOrder;						//管理等级

	//用户状态
	uint16							wTableID;							//桌子索引
	uint16							wChairID;							//椅子索引
	uint8							cbUserStatus;						//用户状态

	//积分信息
	uint64							lScore;								//用户分数
	uint64							lGrade;								//用户成绩
	uint64							lInsure;							//用户银行
	uint64							lIngot;								//用户元宝
	uint64							dBeans;								//用户游戏豆
	bool							bAndroid;							//是否为机器人
	
																		//游戏信息
	uint32							dwWinCount;							//胜利盘数
	uint32							dwLostCount;						//失败盘数
	uint32							dwDrawCount;						//和局盘数
	uint32							dwFleeCount;						//逃跑盘数
	uint32							dwExperience;						//用户经验
	uint32							lLoveLiness;						//用户魅力
	uint64							lIntegralCount;						//积分总数(当前房间)

	//代理信息
	uint32							dwAgentID;							//代理 I D
};

//用户信息
struct CMD_GR_MobileUserInfoHead
{
	//用户属性
	uint32							dwGameID;							//游戏 I D
	uint32							dwUserID;							//用户 I D

	//头像信息
	uint16							wFaceID;							//头像索引
	uint32							dwCustomID;							//自定标识

	wchar_t							szNickName[LEN_NICKNAME];			//玩家昵称

	//用户属性
	uint8							cbGender;							//用户性别
	uint8							cbMemberOrder;						//会员等级

	//用户状态
	uint16							wTableID;							//桌子索引
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


//坐下请求
struct CMD_GR_UserSitDown
{
	uint16							wTableID;							//桌子位置
	uint16							wChairID;							//椅子位置
	wchar_t							szPassword[LEN_PASSWORD];			//桌子密码
};

//用户状态
struct CMD_GR_UserStatus
{
	uint32							dwUserID;							//用户标识
	tagUserStatus					UserStatus;							//用户状态
};

//请求失败
struct CMD_GR_UserRequestFailure
{
	uint32							lErrorCode;							//错误代码
	wchar_t							szDescribeString[256];				//描述信息
};

//////////////////////////////////////////////////////////////////////////////////
//状态命令

#define MDM_GR_STATUS				4									//状态信息

#define SUB_GR_TABLE_INFO			100									//桌子信息
#define SUB_GR_TABLE_STATUS			101									//桌子状态

//////////////////////////////////////////////////////////////////////////////////
//桌子状态
struct tagTableStatus
{
	uint8							cbTableLock;						//锁定标志
	uint8							cbPlayStatus;						//游戏标志
	uint32							lCellScore;							//单元积分
};

//桌子信息
struct CMD_GR_TableInfo
{
	uint16							wTableCount;						//桌子数目
	tagTableStatus					TableStatusArray[512];				//桌子状态
};

//桌子状态
struct CMD_GR_TableStatus
{
	uint16							wTableID;							//桌子号码
	tagTableStatus					TableStatus;						//桌子状态
};


//////////////////////////////////////////////////////////////////////////////////
//框架命令
#define MDM_GF_FRAME				100									//框架命令

#define SUB_GF_GAME_OPTION			1									//游戏配置
#define SUB_GF_GAME_STATUS			100									//游戏状态
#define SUB_GF_GAME_SCENE			101									//游戏场景
#define SUB_GF_USER_DATA			103									//玩家数据

#define SUB_GF_BJL_GAME_STATUS		2001								//房间状态
#define SUB_GF_BJL_GAME_RESULT		2002								//百家乐通知房间玩家单局游戏结果
#define SUB_GF_BJL_CHANGE_STATUS	2003								//百家乐通知房间玩家游戏状态改变
#define SUB_GF_BAIREN_STATUS_END	2004								//请求获取列表完成

//游戏配置
struct CMD_GF_GameOption
{
	uint8							cbAllowLookon;						//旁观标志
	uint32							dwFrameVersion;						//框架版本
	uint32							dwClientVersion;					//游戏版本
};

//游戏环境
struct CMD_GF_GameStatus
{
	uint8							cbGameStatus;						//游戏状态
	uint8							cbAllowLookon;						//旁观标志
};

//游戏环境
struct CMD_GF_GameUserData
{
	uint32							cbUserCharID;						//游戏状态
};

//游戏状态切换
struct tagChangeStatus
{
	uint16							wTableID;							//桌子ID
	uint8							cbGameStatus;						//游戏状态
};

//红黑时间信息
struct tagRBTimeInfo
{
	uint8							cbBetTime;							//下注时间
	uint8							cbEndTime;							//结束时间
	uint8							cbPassTime;							//已过时间

	uint64							lMinXianHong;						//最小限红
	uint64							lMaxXianHong;						//最大限红
};

//红黑记录信息
struct tagRBGameRecord
{
	uint8							cbAreaWin[4];							//区域胜利标识
};

//红黑房间状态
struct CMD_GF_RBRoomStatus
{
	tagChangeStatus					tagGameInfo;						//房间信息
	tagRBTimeInfo					tagTimeInfo;						//时间信息
	tagRBGameRecord					GameRecordArrary[48];				//路单记录
	uint8							cbRecordCount;						//记录条数
};


//////////////////////////////////////////////////////////////////////////////////
//游戏命令

#define MDM_GF_GAME					200									//游戏命令

#pragma pack()

#endif