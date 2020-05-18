#ifndef CMD_RED_BLACK_H
#define CMD_RED_BLACK_H

#include "Define.h"
#include "CMD_GameServer.h"

#pragma pack(1)

namespace SubGame
{
	enum RedBlack
	{
		//索引定义
		INDEX_PLAYER			=	0	,								//红家索引
		INDEX_BANKER			=	1	,								//黑家索引


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
		GAME_SCENE_BET			=	GAME_STATUS_FREE,					//下注状态
		GAME_SCENE_BET_FINISH	=	GAME_STATUS_FREE+1,					//下注状态
		GAME_SCENE_END			=	GAME_STATUS_PLAY,					//结束状态
	};

	enum SubGameTimerID
	{
		//下注时间
		IDI_PLACE_JETTON		=	2,									//下注时间

		IDI_GAME_END			=	3,									//结束时间
	};
	
	struct tagServerGameRecord
	{
		bool							bPlayer;
		bool							bBanker;
		uint8							cbPing;
		uint8							cbCardType;  //1单张，2对子,3顺子,4金花,5顺金,6豹子
	};

#define SUB_S_GAME_FREE				2002									//游戏空闲
#define SUB_S_GAME_START			2003									//游戏开始
#define SUB_S_PLACE_JETTON			2004									//用户下注
#define SUB_S_PLACE_JETTON_FAIL		2005									//下注失败
#define SUB_S_GAME_END				2006									//游戏结束

#define SUB_S_ONLINE_PLAYER			2014									//在线用户
#define SUB_S_SEND_RECORD			2017									//游戏记录
#define SUB_S_CHEHUI				2019									//下注撤回

#define SUB_S_OTHER_JETTON			2021									//其它玩家下注
#define SUB_S_SEAT_JETTON			2022									//占位玩家下注

	//游戏状态
	struct CMD_S_StatusPlay
	{
		//全局信息
		uint8							cbTimeLeave;						//剩余时间

		int								nChip[MAX_CHIP_COUNT];				//筹码配置

		uint16							wApplyUser[GAME_PLAYER];			//上庄列表玩家的GameID

		uint16							wSeatUser[MAX_SEAT_COUNT];	//6个椅子玩家的椅子号

		SCORE							lSeatUserAreaScore[MAX_SEAT_COUNT][AREA_MAX];	//6个椅子玩家的区域下注信息

		uint16							wBankerUser;						//当前庄家的GameID
		SCORE							lBankerScore;						//庄家分数
		SCORE							lAreaLimitScore;					//区域限制
		SCORE							lApplyBankerCondition;				//申请条件

		uint8							cbFreeTime;							//空闲时间
		uint8							cbPlayTime;							//游戏时间
		uint8							cbEndTime;							//结束时间

		SCORE							lMinXianHong;
		SCORE							lMaxXianHong;

		SCORE							lPlayerJettonScore[AREA_MAX];		//玩家下的注
		SCORE							lAllJettonScore[AREA_MAX];			//所有玩家下的注

		uint8							cbTableCardArray[CARD_COUNT][AREA_MAX];		//桌面扑克

		uint8							cbResult[AREA_MAX + 1];					//结果  0:龙 1虎 2和

		SCORE							lBankerWinScore;					//庄家赢分
		SCORE							lPlayerWinScore;					//玩家赢分
		SCORE							lSeatUserWinScore[MAX_SEAT_COUNT];	//坐下的玩家输赢
		SCORE							lPlayerRestScore;					//玩家还剩多少钱
		SCORE							lBankerRestScore;					//庄家还剩多少钱
		SCORE							SeatPlayerRestScore[MAX_SEAT_COUNT];
	};


	//游戏开始
	struct CMD_S_GameStart
	{
		uint8							cbBetTime;							//剩余时间
		uint16							wSeatUser[MAX_SEAT_COUNT];			
	};

	//失败结构
	struct CMD_S_PlaceBetFail
	{
		wchar							szBuffer[64];						//描述信息
	};

	//用户下注
	struct CMD_S_PlaceBet
	{
		uint16							wChairID;							//用户位置
		uint8							cbBetArea;							//筹码区域
		SCORE							lBetScore;							//加注数目
		SCORE							lPlayerRestScore;					//下注玩家剩余金币
	};

	//游戏结束
	struct CMD_S_GameEnd
	{
		SCORE							lPlayAreaScore[3];			//玩家成绩
		SCORE							lPlayerWinScore;					//玩家赢的钱
		SCORE							lPlayerRestScore;					//玩家还剩多少钱
		SCORE							lBankerWinScore;					//庄家成绩
		SCORE							lBankerRestScore;					//庄家还剩多少钱

		uint16							wSeatUser[6];			//3个椅子玩家的椅子号
		SCORE							lSeatUserWinScore[6];	//坐下的玩家输赢
		SCORE							lPlayerJettonScore[3];		//玩家下的注
		SCORE							lAllJettonScore[3];			//所有玩家下的注
		SCORE							lSeatUserRestScore[6];	//坐下的玩家还剩多少钱

		uint8							cbTableCardArray[2][3];		//桌面扑克
		uint8							cbResult[3 + 1];				//结果
	};

	//玩家列表
	struct CMD_S_UserList
	{
		uint16							wCount;								//数组数量
		bool							bEnd;								//是否结束
		uint8							cbIndex[USER_LIST_COUNT];			//排名
		wchar							szUserNick[USER_LIST_COUNT][32];	//昵称
		SCORE							lBetScore[USER_LIST_COUNT];			//近20局下注金额
		uint8							cbWinTimes[USER_LIST_COUNT];		//近20局赢了多少局
		SCORE							lUserScore[USER_LIST_COUNT];		//玩家金币
		uint8							wFaceID[USER_LIST_COUNT];			//玩家头像
		uint16							wChairID[USER_LIST_COUNT];
	};

	//玩家列表单个数据
	struct CMD_S_UserListInfo
	{
		uint16							wWinNum;							//获胜次数
		SCORE							lAllBet;							//下注分数
		SCORE							lUserScore;							//用户积分
		wchar							szNickName[32];						//用户昵称
		uint8							wFaceID;							//玩家头像
		uint16							wChairID;
	};

	//用户撤回
	struct CMD_S_CheHui
	{
		uint16							wChairID;							//撤回的用户
		SCORE							lUserRestScore;						//用户剩余金币
		SCORE							SurplusChip[AREA_MAX];				//区域剩余金币
	};

	//////////////////////////////////////////////////////////////////////////
	//客户端命令结构
#define SUB_C_PLACE_JETTON			100									//用户下注
#define SUB_C_APPLY_BANKER			101									//申请庄家
#define SUB_C_CANCEL_BANKER			102									//取消申请
#define SUB_C_ONLINE_PLAYER			103									//在线用户
#define SUB_C_CHEHUI				105									//撤回下注

	//用户下注
	struct CMD_C_PlaceBet
	{
		uint8							cbBetArea;							//筹码区域
		SCORE							lBetScore;							//加注数目
	};
}

#pragma pack()

#endif