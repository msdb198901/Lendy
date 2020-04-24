#ifndef TABLE_FRAME_SINK_H
#define TABLE_FRAME_SINK_H

#include "CMD_RedBlack.h"
#include "GameServerManager.h"

namespace SubGame
{
	class CTableFrameSink : public ITableFrameSink, public ITableUserAction
	{
		//函数定义
	public:
		//构造函数
		CTableFrameSink();
		//析构函数
		virtual ~CTableFrameSink();

		//基础接口
	public:
		//释放对象
		virtual void Release();
		//接口查询
		virtual void * QueryInterface(GGUID Guid);

		//管理接口
	public:
		//复位接口
		virtual void RepositionSink();
		//配置接口
		virtual bool Initialization(IUnknownEx * pIUnknownEx);

		//动作事件
	public:
		//用户断线
		virtual bool OnActionUserOffLine(uint16 wChairID, IRoomUserItem * pIServerUserItem);
		//用户重入
		virtual bool OnActionUserConnect(uint16 wChairID, IRoomUserItem * pIServerUserItem);
		//用户坐下
		virtual bool OnActionUserSitDown(uint16 wChairID, IRoomUserItem * pIServerUserItem, bool bLookonUser);
		//用户起来
		virtual bool OnActionUserStandUp(uint16 wChairID, IRoomUserItem * pIServerUserItem, bool bLookonUser);
		//用户同意
		virtual bool OnActionUserOnReady(uint16 wChairID, IRoomUserItem * pIServerUserItem, void * pData, uint16 wDataSize);

		//游戏事件
	public:
		//游戏开始
		virtual bool OnEventGameStart();
		//游戏结束
		virtual bool OnEventGameConclude(uint16 wChairID, IRoomUserItem * pIServerUserItem, uint8 cbReason);
		//发送场景
		virtual bool OnEventSendGameScene(uint16 wChairID, IRoomUserItem * pIServerUserItem, uint8 cbGameStatus, bool bSendSecret);
		//下注状态
		virtual bool OnEventGetBetStatus(uint16 wChairID, IRoomUserItem * pIServerUserItem);
		//游戏记录
		virtual void OnGetGameRecord(void *GameRecord);

		//网络接口
	public:
		//游戏消息
		virtual bool OnGameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem);
		//框架消息
		virtual bool OnFrameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem);

		//事件接口
	public:
		//时间事件
		virtual bool OnTimerMessage(uint32 dwTimerID);

		//游戏事件
	protected:
		//用户列表
		bool OnGetUserListGameID(uint16 wSeatUser[MAX_SEAT_COUNT]);

		//辅助函数
	private:
		//读取配置
		void ReadConfigInformation();

		//组件变量
	protected:
		ITableFrame	*					m_pITableFrame;							//框架接口
		tagGameServiceOption*			m_pGameServiceOption;
	
		//用户信息
	protected:
		uint64							m_lUserStartScore[GAME_PLAYER];						//起始分数

		bool							m_bUserListWin[GAME_PLAYER][USER_LIST_COUNT];		//获胜次数
		uint16							m_wUserPlayCount[GAME_PLAYER];						//游戏局数
		uint64							m_lUserListScore[GAME_PLAYER][USER_LIST_COUNT];		//下注数目

		//下注数
	protected:
		uint64							m_lAllBet[AREA_MAX];					//总下注量
		uint64							m_lPlayBet[GAME_PLAYER][AREA_MAX];		//玩家下注
		uint32							m_nChip[MAX_CHIP_COUNT];				//筹码配置

		//分数
	protected:
		uint64							m_lBankerScore;							//庄家积分
		uint64							m_lPlayScore[GAME_PLAYER][AREA_MAX];	//玩家输赢
		uint64							m_lUserWinScore[GAME_PLAYER];			//玩家成绩
		uint64							m_lUserRevenue[GAME_PLAYER];			//玩家税收

		//时间设置
	protected:
		uint8							m_cbFreeTime;							//空闲时间
		uint8							m_cbBetTime;							//下注时间
		uint8							m_cbEndTime;							//结束时间

		//扑克信息
	protected:
		uint8							m_cbCardCount[CARD_COUNT];					//扑克数目
		uint8							m_cbTableCardArray[CARD_COUNT][AREA_MAX];	//桌面扑克
		uint8							m_cbOpenResult[AREA_MAX + 1];				//开奖结果

		//状态变量
	protected:
		uint32							m_dwBetTime;							//开始时间

		//庄家信息
	protected:
		std::vector<uint16>				m_ApplyUserArray;						//申请玩家
		uint16							m_wCurrentBanker;						//当前庄家
		uint16							m_wOfflineBanker;						//离线庄家
		uint16							m_wBankerTime;							//做庄次数

		uint64							m_lBankerWinScore;						//累计成绩
		uint64							m_lBankerCurGameScore;					//当前成绩

		bool							m_bEnableSysBanker;						//系统做庄

		//庄家设置
protected:
		uint64							m_nBankerTimeLimit;						//最大庄家数
		uint64							m_nBankerTimeAdd;						//庄家增加数
		uint64							m_lExtraBankerScore;					//庄家钱
		uint64							m_nExtraBankerTime;						//庄家钱大时,坐庄增加数

		uint64							m_lPlayerBankerMAX;						//玩家最大庄家数
		bool							m_bExchangeBanker;						//交换庄家

		//控制变量
protected:
		uint64							m_lAreaLimitScore;						//区域限制
		uint64							m_lUserLimitScore;						//区域限制
		uint64							m_lApplyBankerCondition;				//申请条件

		//记录变量
	protected:
		tagServerGameRecord				m_GameRecordArrary[MAX_SCORE_HISTORY];	//游戏记录
		uint32							m_dwRecordFirst;						//开始记录
		uint32							m_dwRecordLast;							//最后记录
		uint32							m_dwRecordCount;						//记录数目
	};
}


#endif