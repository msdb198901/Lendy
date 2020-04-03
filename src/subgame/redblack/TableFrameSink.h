#ifndef TABLE_FRAME_SINK_H
#define TABLE_FRAME_SINK_H

#include "GameServerManager.h"

namespace SubGame
{
	class CTableFrameSink : public ITableFrameSink
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

		//游戏事件
	public:
		//游戏开始
		virtual bool OnEventGameStart();
		//游戏结束
		virtual bool OnEventGameConclude(uint16 wChairID, IServerUserItem * pIServerUserItem, uint8 cbReason);
		//发送场景
		virtual bool OnEventSendGameScene(uint16 wChairID, IServerUserItem * pIServerUserItem, uint8 cbGameStatus, bool bSendSecret);

		//网络接口
	public:
		//游戏消息
		virtual bool OnGameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IServerUserItem * pIServerUserItem);
		//框架消息
		virtual bool OnFrameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IServerUserItem * pIServerUserItem);
	};
}


#endif