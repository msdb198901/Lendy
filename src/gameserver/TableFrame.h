#ifndef TABLE_FRAME_H
#define TABLE_FRAME_H

#include "GameComponent.h"

namespace Game
{
	class CTableFrame : public ITableFrame
	{
		//数组定义
		typedef IRoomUserItem * CTableUserItemArray[MAX_CHAIR];				//游戏数组

		//函数定义
	public:
		//构造函数
		CTableFrame();
		//析构函数
		virtual ~CTableFrame();

		virtual void Release() { delete this; }
		virtual void *QueryInterface(GGUID uuid);

		//属性接口
	public:
		//桌子号码
		virtual uint16 GetTableID();
		//游戏人数
		virtual uint16 GetChairCount();
		//空位置数目
		virtual uint16 GetNullChairCount();

		//用户接口
	public:
		//寻找用户
		virtual IRoomUserItem * SearchUserItem(uint32 dwUserID);
		//游戏用户
		virtual IRoomUserItem * GetTableUserItem(uint32 wChairID);
		//查找用户
		virtual IRoomUserItem * SearchUserItemGameID(uint32 dwGameID);

		//信息接口
	public:
		//游戏状态
		virtual bool IsGameStarted();
		//游戏状态
		virtual bool IsDrawStarted();
		//游戏状态
		virtual bool IsTableStarted();

		virtual void SetGameStarted(bool cbGameStatus);

		//控制接口
	public:
		//开始游戏
		virtual bool StartGame();
		//解散游戏
		virtual bool DismissGame();
		//结束游戏
		virtual bool ConcludeGame(uint8 cbGameStatus) ;
		//结束桌子
		virtual bool ConcludeTable();

		//写分接口
	public:
		//写入积分
		virtual bool WriteUserScore(uint8 wChairID, SCORE & lScore);
		//写入积分
		virtual bool WriteTableScore(SCORE ScoreArray[], uint16 wScoreCount);

		//功能接口
	public:
		//发送场景
		virtual bool SendGameScene(IRoomUserItem * pIServerUserItem, void * pData, uint16 wDataSize);

		//时间接口
	public:
		//设置时间
		virtual bool SetGameTimer(uint32 dwTimerID, uint32 dwElapse, uint32 dwRepeat);
		//删除时间
		virtual bool KillGameTimer(uint32 dwTimerID);

		//状态接口
	public:
		//获取状态
		virtual uint8 GetGameStatus();
		//设置状态
		virtual void SetGameStatus(uint8 bGameStatus);

		//游戏用户
	public:
		//发送数据
		virtual bool SendTableData(uint16 wChairID, uint16 wSubCmdID, void * pData = nullptr, uint16 wDataSize = 0, uint16 wMainCmdID = MDM_GF_GAME);

		//动作处理
	public:
		//起立动作
		virtual bool PerformStandUpAction(IRoomUserItem * pIServerUserItem, bool bInitiative = false);
		//坐下动作
		virtual bool PerformSitDownAction(uint16 wChairID, IRoomUserItem * pIServerUserItem, const char* szPassword = nullptr);

		//配置接口
	public:
		//开始模式
		virtual uint8 GetStartMode() { return m_cbStartMode; }
		//开始模式
		virtual void SetStartMode(uint8 cbStartMode) { m_cbStartMode = cbStartMode; }

		//状态接口
	public:
		//获取配置
		virtual tagGameServiceOption* GetGameServiceOption();

		//用户事件
	public:
		//断线事件
		bool OnEventUserOffLine(IRoomUserItem * pIServerUserItem);

		//系统事件
	public:
		//时间事件
		bool OnEventTimer(uint32 dwTimerID);
		//游戏事件
		bool OnEventSocketGame(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem);
		//框架事件
		bool OnEventSocketFrame(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem);

		//效验函数
	public:
		//开始效验
		bool EfficacyStartGame(uint16 wReadyChairID);

	public:
		//配置桌子
		bool InitializationFrame(uint16 wTableID, tagTableFrameParameter & TableFrameParameter);

		//功能函数
	public:
		//游戏记录
		void OnGetGameRecord(void *GameRecord);

		//功能函数
	public:
		//获取空位
		uint16 GetNullChairID();
		//用户数目
		uint16 GetSitUserCount();

		//辅助函数
	public:
		//桌子状态
		bool SendTableStatus();
	
		//游戏属性
	protected:
		uint8							m_cbStartMode;
		uint8							m_cbGameStatus;
		uint16							m_wTableID;							//桌子号码
		uint16							m_wChairCount;						//椅子数目
		uint16							m_wUserCount;						//用户数目
		uint16							m_wPlayCount;						//游戏局数

		//状态变量
	protected:
		bool							m_bGameStarted;						//游戏标志
		bool							m_bDrawStarted;						//游戏标志
		bool							m_bTableStarted;					//游戏标志
		bool							m_bTableInitFinish;					//初始标识

		uint32							m_dwDrawStartTime;

		//用户数组
	protected:
		CTableUserItemArray				m_TableUserItemArray;				//游戏用户
	
		//数据接口
	protected:
		Net::ITimerEngine *				m_pITimerEngine;					//时间引擎
		ITableFrameSink	*				m_pITableFrameSink;					//桌子接口
		IMainServiceFrame *				m_pIMainServiceFrame;				//服务接口

		//扩展接口
	protected:
		ITableUserAction *				m_pITableUserAction;				//动作接口

	//配置信息
	protected:
		tagGameServiceOption *			m_pGameServiceOption;					//配置参数
	};
}

#endif