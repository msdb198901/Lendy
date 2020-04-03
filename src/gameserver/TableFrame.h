#ifndef TABLE_FRAME_H
#define TABLE_FRAME_H

#include "GameComponent.h"

namespace Game
{
	class CTableFrame
	{
		//函数定义
	public:
		//构造函数
		CTableFrame();
		//析构函数
		virtual ~CTableFrame();

	public:
		//配置桌子
		bool InitializationFrame(uint16 wTableID, tagTableFrameParameter & TableFrameParameter);
	
		//游戏属性
	protected:
		uint16							m_wTableID;							//桌子号码
		uint16							m_wChairCount;						//椅子数目
	
		//数据接口
	protected:
		Net::ITCPSocketService *		m_pITCPSocketService;				//网络服务
		ITableFrameSink	*				m_pITableFrameSink;					//桌子接口

	//配置信息
	protected:
		tagGameServiceOption *			m_pGameServiceOption;					//配置参数
	};
}

#endif