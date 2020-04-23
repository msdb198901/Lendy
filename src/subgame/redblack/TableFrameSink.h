#ifndef TABLE_FRAME_SINK_H
#define TABLE_FRAME_SINK_H

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


		//组件变量
	protected:
		ITableFrame	*					m_pITableFrame;							//框架接口
	};
}


#endif