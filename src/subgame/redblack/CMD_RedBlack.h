#ifndef CMD_RED_BLACK_H
#define CMD_RED_BLACK_H

#include "Define.h"
#include "CMD_GameServer.h"

#pragma pack(1)

namespace SubGame
{
	enum RedBlack
	{
		CARD_COUNT = 2,
		MAX_CHIP_COUNT = 5,
		USER_LIST_COUNT = 20,
		MAX_SCORE_HISTORY = 72,

		MAX_SEAT_COUNT = 6,

		AREA_XIAN			=		0,									//黑
		AREA_ZHUANG			=		1,									//红
		AREA_PING			=		2,									//幸运一击
		AREA_MAX			=		3,									//最大区域

		TIME_FREE = 1,
		TIME_PLACE_JETTON = 5,
		TIME_GAME_END = 8,

		GAME_PLAYER			=		100									//游戏人数
	};

	enum  GameSubMsgID
	{
		SUB_S_GAME_START		=	2003,									//游戏开始
		SUB_S_GAME_END			=	2006,									//游戏结束
	};

	enum GameStatus
	{
		GAME_SCENE_FREE			=	GAME_STATUS_FREE,					//等待开始
		GAME_SCENE_BET			=	GAME_STATUS_FREE,					//下注状态
		GAME_SCENE_END			=	GAME_STATUS_PLAY,					//结束状态
	};

	enum SubGameTimerID
	{
		//下注时间
		IDI_PLACE_JETTON		=	2,									//下注时间
	};
	
	struct tagServerGameRecord
	{
		bool							bPlayer;
		bool							bBanker;
		uint8							cbPing;
		uint8							cbCardType;  //1单张，2对子,3顺子,4金花,5顺金,6豹子
	};

	//游戏状态
	struct CMD_S_StatusPlay
	{
		//全局信息
		uint8							cbTimeLeave;						//剩余时间

		int								nChip[MAX_CHIP_COUNT];				//筹码配置

		uint16							wApplyUser[GAME_PLAYER];			//上庄列表玩家的GameID

		uint16							wSeatUser[MAX_SEAT_COUNT];	//6个椅子玩家的椅子号

		uint64							lSeatUserAreaScore[MAX_SEAT_COUNT][AREA_MAX];	//6个椅子玩家的区域下注信息

		uint16							wBankerUser;						//当前庄家的GameID
		uint64							lBankerScore;						//庄家分数
		uint64							lAreaLimitScore;					//区域限制
		uint64							lApplyBankerCondition;				//申请条件

		uint8							cbFreeTime;							//空闲时间
		uint8							cbPlayTime;							//游戏时间
		uint8							cbEndTime;							//结束时间

		uint64							lMinXianHong;
		uint64							lMaxXianHong;

		uint64							lPlayerJettonScore[AREA_MAX];		//玩家下的注
		uint64							lAllJettonScore[AREA_MAX];			//所有玩家下的注

		uint8							cbTableCardArray[CARD_COUNT][AREA_MAX];		//桌面扑克

		uint8							cbResult[AREA_MAX + 1];					//结果  0:龙 1虎 2和

		uint64							lBankerWinScore;					//庄家赢分
		uint64							lPlayerWinScore;					//玩家赢分
		uint64							lSeatUserWinScore[MAX_SEAT_COUNT];	//坐下的玩家输赢
		uint64							lPlayerRestScore;					//玩家还剩多少钱
		uint64							lBankerRestScore;					//庄家还剩多少钱
		uint64							SeatPlayerRestScore[MAX_SEAT_COUNT];
	};


	//游戏开始
	struct CMD_S_GameStart
	{
		uint8							cbBetTime;							//剩余时间
		uint16							wSeatUser[MAX_SEAT_COUNT];			
	};


	//游戏结束
	struct CMD_S_GameEnd
	{
		uint64							lPlayAreaScore[AREA_MAX];			//玩家成绩
		uint64							lPlayerWinScore;					//玩家赢的钱
		uint64							lPlayerRestScore;					//玩家还剩多少钱
		uint64							lBankerWinScore;					//庄家成绩
		uint64							lBankerRestScore;					//庄家还剩多少钱

		uint16							wSeatUser[MAX_SEAT_COUNT];			//3个椅子玩家的椅子号
		uint64							lSeatUserWinScore[MAX_SEAT_COUNT];	//坐下的玩家输赢
		uint64							lPlayerJettonScore[AREA_MAX];		//玩家下的注
		uint64							lAllJettonScore[AREA_MAX];			//所有玩家下的注
		uint64							lSeatUserRestScore[MAX_SEAT_COUNT];	//坐下的玩家还剩多少钱

		uint8							cbTableCardArray[CARD_COUNT][AREA_MAX];				//桌面扑克
		uint8							cbResult[AREA_MAX + 1];				//结果
	};


	//玩家列表单个数据
	struct CMD_S_UserListInfo
	{
		uint16							wWinNum;							//获胜次数
		uint64							lAllBet;							//下注分数
		uint64							lUserScore;							//用户积分
		wchar_t							szNickName[32];						//用户昵称
		uint8							wFaceID;							//玩家头像
		uint16							wChairID;
	};

	//////////////////////////////////////////////////////////////////////////
	//客户端命令结构
#define SUB_C_PLACE_JETTON			100									//用户下注
}

#pragma pack()

#endif