#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "Define.h"

namespace  SubGame
{
#define MAX_COUNT					3									//最大数目
#define	DRAW						2									//和局类型

	//数值掩码
#define	LOGIC_MASK_COLOR			0xF0								//花色掩码
#define	LOGIC_MASK_VALUE			0x0F								//数值掩码

//扑克类型
#define CT_SINGLE					1									//单牌类型
#define CT_DOUBLE					2									//对子类型
#define CT_DOUBLE_SHINE				3									//大对子类
#define	CT_SHUN_ZI					4									//顺子类型
#define CT_JIN_HUA					5									//金花类型
#define	CT_SHUN_JIN					6									//顺金类型
#define	CT_BAO_ZI					7									//豹子类型
#define CT_SPECIAL					8									//特殊类型

	//游戏逻辑类
	class CGameLogic
	{
		//变量定义
	private:
		static uint8						m_cbCardListData[52];				//扑克定义

		//函数定义
	public:
		//构造函数
		CGameLogic();
		//析构函数
		virtual ~CGameLogic();

		//类型函数
	public:
		//获取类型
		uint8 GetCardType(uint8 cbCardData[], uint8 cbCardCount);
		//获取数值
		uint8 GetCardValue(uint8 cbCardData) { return cbCardData & LOGIC_MASK_VALUE; }
		//获取花色
		uint8 GetCardColor(uint8 cbCardData) { return cbCardData & LOGIC_MASK_COLOR; }

		//控制函数
	public:
		//排列扑克
		void SortCardList(uint8 cbCardData[], uint8 cbCardCount);
		//混乱扑克
		void RandCardList(uint8 cbCardBuffer[], uint8 cbBufferCount);

		//功能函数
	public:
		//逻辑数值
		uint8 GetCardLogicValue(uint8 cbCardData);
		//对比扑克
		uint8 CompareCard(uint8 cbFirstData[], uint8 cbNextData[], uint8 cbCardCount);
	};
}

#endif