#ifndef GAME_COMPONENT_H
#define GAME_COMPONENT_H

#include "Define.h"
#include "Struct.h"
#include "KernelEngineHead.h"
#include "CMD_GameServer.h"
#include <vector>

//结构体
namespace Game
{
	//创建函数
#define SUB_GAME_CREATE_NAME	"CreateGameServiceManager"			//创建函数

	struct IGameServiceManager;
	struct IMainServiceFrame;

	//服务属性
	struct tagGameServiceOption
	{
		//内核属性
		uint16							wKindID;							//名称号码
		uint16							wServerID;							//房间标识
		uint16							wServerPort;						//服务端口
		uint16							wChairCount;						//椅子数目

		//功能标志
		uint8							cbDynamicJoin;						//动态加入
		uint8							cbOffLineTrustee;					//断线代打

		//税收配置
		SCORE							lCellScore;							//单位积分
		uint16							wRevenueRatio;						//税收比例
		SCORE							lServiceScore;						//服务费用

		//房间配置
		SCORE							lMinEnterScore;						//最低积分
		SCORE							lMaxEnterScore;						//最高积分

		//房间属性
		uint16							wMaxPlayer;							//最大数目
		uint16							wTableCount;						//桌子数目

		//服务属性
		char							strGameName[32];					//游戏名字
		char							strServerDLLName[32];				//进程名字

		//自定规则
		std::vector<uint8>				vCustomRule;						//游戏规则
	};

	//桌子参数
	struct tagTableFrameParameter
	{
		Net::ITimerEngine *				pITimerEngine;						//时间引擎
		IGameServiceManager *			pIGameServiceManager;				//服务管理
		IMainServiceFrame *				pIMainServiceFrame;					//服务框架
		tagGameServiceOption *			pGameServiceOption;
	};
}

//寻找组件
namespace Game
{
	using namespace Comm;
	static GGUID IID_IGameServiceManager = { 0x39876a3c, 0x7f9a, 0x4ef5, { 0xb2, 0x2a, 0x78, 0x3c, 0x4b, 0x97, 0x84, 0x33 } };
	static GGUID IID_ITableFrameSink = { 0x68b59d58, 0x9153, 0x4102, { 0x89, 0xec, 0x39, 0x79, 0x81, 0x53, 0x9b, 0xa7 } };
	static GGUID IID_IRoomUserItem = { 0x3e5dee5f, 0xcf19, 0x4f86, { 0xbb, 0x9a, 0x81, 0x93, 0x3a, 0xe3, 0x72, 0x42 } };
	static GGUID IID_IRoomUserManager = { 0x962b7164, 0xcaf8, 0x4adb, { 0xbb, 0xdb, 0x4, 0x64, 0x9f, 0xfe, 0xd0, 0xdd } };
	static GGUID IID_IRoomUserItemSink = { 0xdfc40ba8, 0x35df, 0x489b, { 0x9b, 0x9f, 0xb7, 0xcf, 0x1c, 0x67, 0xe9, 0x30 } };
	static GGUID IID_IMainServiceFrame = { 0xef49c080, 0xc4e7, 0x4584, { 0xa0, 0x81, 0x9b, 0x45, 0x27, 0x1e, 0x2c, 0x92 } };
	static GGUID IID_ITableFrame = { 0x9c1a419a, 0xf3db, 0x4d3f, { 0xb7, 0xd2, 0x2, 0x5d, 0x49, 0x64, 0x53, 0xb5 } };
	static GGUID IID_ITableUserAction = { 0x12a14343, 0xd6e2, 0x4852, { 0x81, 0xf1, 0xdb, 0x93, 0x32, 0x84, 0xb2, 0xf0 } };

	//游戏接口
	struct IGameServiceManager : public IUnknownEx
	{
		//创建接口
	public:
		//创建桌子
		virtual void * CreateTableFrameSink(GGUID Guid) = 0;
		//创建机器
		virtual void * CreateAndroidUserItemSink(GGUID Guid) = 0;
		//创建数据
		virtual void * CreateGameDataBaseEngineSink(GGUID Guid) = 0;

		//参数接口
	public:
		//调整参数
		virtual bool RectifyParameter(tagGameServiceOption & GameServiceOption) = 0;
	};

	//用户接口
	struct IRoomUserItem : public IUnknownEx
	{
		//属性信息
	public:
		//用户索引
		virtual uint16 GetBindIndex() = 0;
		//用户地址
		virtual uint64 GetClientAddr() = 0;
		//机器标识
		virtual char* GetMachineID() = 0;

		//登录信息
	public:
		//请求标识
		virtual uint64 GetDBQuestID() = 0;
		//登录时间
		virtual uint64 GetLogonTime() = 0;
		//记录索引
		virtual uint64 GetInoutIndex() = 0;

		//用户信息
	public:
		//用户信息
		virtual tagUserInfo * GetUserInfo() = 0;

		//属性信息
	public:
		//用户性别
		virtual uint8 GetGender() = 0;
		//用户标识
		virtual uint32 GetUserID() = 0;
		//游戏标识
		virtual uint32 GetGameID() = 0;
		//用户昵称
		virtual char* GetNickName() = 0;

		//状态接口
	public:
		//桌子号码
		virtual uint16 GetTableID() = 0;
		//桌子号码
		virtual uint16 GetLastTableID() = 0;
		//椅子号码
		virtual uint16 GetChairID() = 0;
		//用户状态
		virtual uint8 GetUserStatus() = 0;
		//解除绑定
		virtual bool DetachBindStatus() = 0;

		//积分信息
	public:
		//用户积分
		virtual SCORE GetUserScore() = 0;

		//积分信息
	public:
		//用户胜率
		virtual uint16 GetUserWinRate() = 0;
		//用户输率
		virtual uint16 GetUserLostRate() = 0;
		//用户和率
		virtual uint16 GetUserDrawRate() = 0;
		//用户逃率
		virtual uint16 GetUserFleeRate() = 0;
		//游戏局数
		virtual uint16 GetUserPlayCount() = 0;

		//效验接口
	public:
		//对比密码
		virtual bool ContrastLogonPass(const char* szPassword) = 0;

		//托管状态
	public:
		//判断状态
		virtual bool IsTrusteeUser() = 0;
		//设置状态
		virtual void SetTrusteeUser(bool bTrusteeUser) = 0;

		//游戏状态
	public:
		//连接状态
		virtual bool IsClientReady() = 0;
		//设置连接
		virtual void SetClientReady(bool bClientReady) = 0;

		//管理接口
	public:
		//设置状态
		virtual bool SetUserStatus(uint8 cbUserStatus, uint16 wTableID, uint16 wChairID) = 0;

		//高级接口
	public:
		//设置参数
		virtual bool SetUserParameter(uint32 dwClientAddr, uint16 wBindIndex, const char szMachineID[LEN_MACHINE_ID], bool bClientReady) = 0;

		//写入积分
		virtual bool WriteUserScore(SCORE & lScore) = 0;
	};

	//桌子接口
	struct ITableFrame : public IUnknownEx
	{
		//属性接口
	public:
		//桌子号码
		virtual uint16 GetTableID() = 0;
		//游戏人数
		virtual uint16 GetChairCount() = 0;
		//空位置数目
		virtual uint16 GetNullChairCount() = 0;

		//用户接口
	public:
		//寻找用户
		virtual IRoomUserItem * SearchUserItem(uint32 dwUserID) = 0;
		//游戏用户
		virtual IRoomUserItem * GetTableUserItem(uint32 wChairID) = 0;
		//查找用户
		virtual IRoomUserItem * SearchUserItemGameID(uint32 dwGameID) = 0;


		//信息接口
	public:
		//游戏状态
		virtual bool IsGameStarted() = 0;
		//游戏状态
		virtual bool IsDrawStarted() = 0;
		//游戏状态
		virtual bool IsTableStarted() = 0;

		virtual void SetGameStarted(bool cbGameStatus) = 0;

		//控制接口
	public:
		//开始游戏
		virtual bool StartGame() = 0;
		//解散游戏
		virtual bool DismissGame() = 0;
		//结束游戏
		virtual bool ConcludeGame(uint8 cbGameStatus) = 0;

		//写分接口
	public:
		//写入积分
		virtual bool WriteUserScore(uint8 wChairID, SCORE & lScore) = 0;
		//写入积分
		virtual bool WriteTableScore(SCORE ScoreArray[], uint16 wScoreCount) = 0;

		//功能接口
	public:
		//发送场景
		virtual bool SendGameScene(IRoomUserItem * pIServerUserItem, void * pData, uint16 wDataSize) = 0;

		//游戏用户
	public:
		//发送数据
		virtual bool SendTableData(uint16 wChairID, uint16 wSubCmdID, void * pData = nullptr, uint16 wDataSize = 0, uint16 wMainCmdID = MDM_GF_GAME) = 0;

		//动作处理
	public:
		//起立动作
		virtual bool PerformStandUpAction(IRoomUserItem * pIServerUserItem, bool bInitiative = false) = 0;
		//坐下动作
		virtual bool PerformSitDownAction(uint16 wChairID, IRoomUserItem * pIServerUserItem, const char* szPassword = nullptr) = 0;

		//时间接口
	public:
		//设置时间
		virtual bool SetGameTimer(uint32 dwTimerID, uint32 dwElapse, uint32 dwRepeat) = 0;
		//删除时间
		virtual bool KillGameTimer(uint32 dwTimerID) = 0;

		//状态接口
	public:
		//获取状态
		virtual uint8 GetGameStatus() = 0;
		//设置状态
		virtual void SetGameStatus(uint8 bGameStatus) = 0;


		//配置接口
	public:
		//开始模式
		virtual uint8 GetStartMode() = 0;
		//开始模式
		virtual void SetStartMode(uint8 cbStartMode) = 0;

		//状态接口
	public:
		//获取配置
		virtual tagGameServiceOption* GetGameServiceOption() = 0;
	};

	//回调接口
	struct ITableFrameSink : public IUnknownEx
	{
		//管理接口
	public:
		//复位接口
		virtual void RepositionSink() = 0;
		//配置接口
		virtual bool Initialization(IUnknownEx * pIUnknownEx) = 0;

		//游戏事件
	public:
		//游戏开始
		virtual bool OnEventGameStart() = 0;
		//游戏结束
		virtual bool OnEventGameConclude(uint16 wChairID, IRoomUserItem * pIServerUserItem, uint8 cbReason) = 0;
		//发送场景
		virtual bool OnEventSendGameScene(uint16 wChairID, IRoomUserItem * pIServerUserItem, uint8 cbGameStatus, bool bSendSecret) = 0;
		//下注状态
		virtual bool OnEventGetBetStatus(uint16 wChairID, IRoomUserItem * pIServerUserItem) = 0;
		//游戏记录
		virtual void OnGetGameRecord(void *GameRecord) = 0;

		//事件接口
	public:
		//时间事件
		virtual bool OnTimerMessage(uint32 dwTimerID) = 0;

		//网络接口
	public:
		//游戏消息
		virtual bool OnGameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem) = 0;
		//框架消息
		virtual bool OnFrameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem) = 0;
	};

	//用户管理
	struct IRoomUserManager : public IUnknownEx
	{
		//配置接口
	public:
		//设置接口
		virtual bool SetServerUserItemSink(IUnknownEx * pIUnknownEx) = 0;

		//查找接口
	public:
		//查找用户
		virtual IRoomUserItem * SearchUserItem(uint32 dwUserID) = 0;
		//查找用户
		virtual IRoomUserItem * SearchUserItem(char* pszNickName) = 0;

		//统计接口
	public:
		//机器人数
		virtual uint32 GetAndroidCount() = 0;
		//在线人数
		virtual uint32 GetUserItemCount() = 0;

		//管理接口
	public:
		//删除用户
		virtual bool DeleteUserItem() = 0;
		//删除用户
		virtual bool DeleteUserItem(IRoomUserItem * pIServerUserItem) = 0;
		//插入用户
		virtual bool InsertUserItem(IRoomUserItem * * pIServerUserResult, tagUserInfo & UserInfo, tagUserInfoPlus &UserInfoPlus) = 0;
	};

	//状态接口
	struct IRoomUserItemSink : public IUnknownEx
	{
		//用户积分
		virtual bool OnEventUserItemScore(IRoomUserItem * pIServerUserItem, uint8 cbReason) = 0;
		//用户状态
		virtual bool OnEventUserItemStatus(IRoomUserItem * pIServerUserItem, uint16 wOldTableID = INVALID_TABLE, uint16 wOldChairID = INVALID_CHAIR) = 0;
	};

	//服务框架
	struct IMainServiceFrame : public IUnknownEx
	{
		//消息接口
	public:
		//房间消息
		virtual bool SendRoomMessage(char* lpszMessage, uint16 wType) = 0;
		//游戏消息
		virtual bool SendGameMessage(char* lpszMessage, uint16 wType) = 0;
		//房间消息
		virtual bool SendRoomMessage(IRoomUserItem * pIServerUserItem, const char* lpszMessage, uint16 wType) = 0;
		//游戏消息
		virtual bool SendGameMessage(IRoomUserItem * pIServerUserItem, const char* lpszMessage, uint16 wType) = 0;
		//房间消息
		virtual bool SendRoomMessage(uint32 dwSocketID, const char* lpszMessage, uint16 wType, bool bAndroid) = 0;

		//发送数据
		virtual bool SendData(uint32 dwSocketID, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize) = 0;
		//发送数据
		virtual bool SendData(IRoomUserItem * pIServerUserItem, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize) = 0;
		//群发数据
		virtual bool SendDataBatch(uint16 wCmdTable, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize) = 0;
		//解锁积分
		virtual void UnLockScoreLockUser(uint32 dwUserID, uint32 dwInoutIndex, uint32 dwLeaveReason) = 0;
	};

	//用户动作
	struct ITableUserAction : public IUnknownEx
	{
		//用户断线
		virtual bool OnActionUserOffLine(uint16 wChairID, IRoomUserItem * pIServerUserItem) = 0;
		//用户重入
		virtual bool OnActionUserConnect(uint16 wChairID, IRoomUserItem * pIServerUserItem) = 0;
		//用户坐下
		virtual bool OnActionUserSitDown(uint16 wChairID, IRoomUserItem * pIServerUserItem, bool bLookonUser) = 0;
		//用户起来
		virtual bool OnActionUserStandUp(uint16 wChairID, IRoomUserItem * pIServerUserItem, bool bLookonUser) = 0;
		//用户同意
		virtual bool OnActionUserOnReady(uint16 wChairID, IRoomUserItem * pIServerUserItem, void * pData, uint16 wDataSize) = 0;
		//用户下注
		virtual bool OnActionUserBet(uint16 wChairID, IRoomUserItem * pIServerUserItem) { return true; }
	};


	DECLARE_MOUDLE_DYNAMIC(GameServiceManager)
}

#endif