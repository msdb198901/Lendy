#ifndef GAME_COMPONENT_H
#define GAME_COMPONENT_H

#include "Define.h"
#include "KernelEngineHead.h"
#include <vector>

//结构体
namespace Game
{
	//创建函数
#define SUB_GAME_CREATE_NAME	"CreateGameServiceManager"			//创建函数

	struct IGameServiceManager;

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
		uint64							lCellScore;							//单位积分
		uint16							wRevenueRatio;						//税收比例
		uint64							lServiceScore;						//服务费用

		//房间配置
		uint64							lMinEnterScore;						//最低积分
		uint64							lMaxEnterScore;						//最高积分

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
		IGameServiceManager *			pIGameServiceManager;				//服务管理
		Net::ITCPSocketService *		pITCPSocketService;					//网络服务
		tagGameServiceOption *			pGameServiceOption;
	};
}

//寻找组件
namespace Game
{
	static GGUID IID_IGameServiceManager = { 0x39876a3c, 0x7f9a, 0x4ef5, { 0xb2, 0x2a, 0x78, 0x3c, 0x4b, 0x97, 0x84, 0x33 } };
	static GGUID IID_ITableFrameSink = { 0x68b59d58, 0x9153, 0x4102, { 0x89, 0xec, 0x39, 0x79, 0x81, 0x53, 0x9b, 0xa7 } };
	static GGUID IID_IServerUserItem = { 0x3e5dee5f, 0xcf19, 0x4f86, { 0xbb, 0x9a, 0x81, 0x93, 0x3a, 0xe3, 0x72, 0x42 } };

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
	struct IServerUserItem : public IUnknownEx
	{
		//属性信息
	public:
		//用户索引
		virtual uint16 GetBindIndex() = 0;
		//用户地址
		virtual uint64 GetClientAddr() = 0;
		//机器标识
		virtual wchar_t* GetMachineID() = 0;

		//登录信息
	public:
		//请求标识
		virtual uint64 GetDBQuestID() = 0;
		//登录时间
		virtual uint64 GetLogonTime() = 0;
		//记录索引
		virtual uint64 GetInoutIndex() = 0;

		//属性信息
	public:
		//用户性别
		virtual uint8 GetGender() = 0;
		//用户标识
		virtual uint64 GetUserID() = 0;
		//游戏标识
		virtual uint64 GetGameID() = 0;
		//用户昵称
		virtual wchar_t* GetNickName() = 0;

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

		//积分信息
	public:
		//用户积分
		virtual uint64 GetUserScore() = 0;

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
		virtual bool OnEventGameConclude(uint16 wChairID, IServerUserItem * pIServerUserItem, uint8 cbReason) = 0;
		//发送场景
		virtual bool OnEventSendGameScene(uint16 wChairID, IServerUserItem * pIServerUserItem, uint8 cbGameStatus, bool bSendSecret) = 0;

		//网络接口
	public:
		//游戏消息
		virtual bool OnGameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IServerUserItem * pIServerUserItem) = 0;
		//框架消息
		virtual bool OnFrameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IServerUserItem * pIServerUserItem) = 0;
	};

	DECLARE_MOUDLE_DYNAMIC(GameServiceManager)
}

#endif