#include "TableFrame.h"
#include "Header.h"
#include "CMD_GameServer.h"
#include "StringFormat.h"

namespace Game
{
	CTableFrame::CTableFrame() : 
		m_wTableID(0),
		m_wChairCount(0),
		m_wUserCount(0),
		m_bGameStarted(false),

		m_pITCPSocketService(nullptr),
		m_pGameServiceOption(nullptr)
	{
		//用户数组
		memset(m_TableUserItemArray, 0, sizeof(m_TableUserItemArray));
	}

	CTableFrame::~CTableFrame()
	{

	}

	IRoomUserItem * CTableFrame::SearchUserItem(uint32 dwUserID)
	{
		return nullptr;
	}

	IRoomUserItem * CTableFrame::GetTableUserItem(uint32 wChairID)
	{
		//效验参数
		assert(wChairID < m_wChairCount);
		if (wChairID >= m_wChairCount) return nullptr;

		//获取用户
		return m_TableUserItemArray[wChairID];
	}

	IRoomUserItem * CTableFrame::SearchUserItemGameID(uint32 dwGameID)
	{
		return nullptr;
	}

	bool CTableFrame::IsGameStarted()
	{
		return m_bGameStarted;
	}

	bool CTableFrame::IsDrawStarted()
	{
		return m_bDrawStarted;
	}

	bool CTableFrame::IsTableStarted()
	{
		return m_bTableStarted;
	}

	void CTableFrame::SetGameStarted(bool cbGameStatus)
	{
	}

	bool CTableFrame::StartGame()
	{
		return false;
	}

	bool CTableFrame::DismissGame()
	{
		return false;
	}

	bool CTableFrame::ConcludeGame(uint8 cbGameStatus)
	{
		return false;
	}

	bool CTableFrame::ConcludeTable()
	{
		return false;
	}

	bool CTableFrame::SendGameScene(IRoomUserItem * pIServerUserItem, void * pData, uint16 wDataSize)
	{
		//用户效验
		assert((pIServerUserItem != nullptr) && pIServerUserItem->IsClientReady());
		if ((pIServerUserItem == nullptr) || !pIServerUserItem->IsClientReady()) return false;

		//发送场景
		assert(m_pIMainServiceFrame != nullptr);
		m_pIMainServiceFrame->SendData(pIServerUserItem, MDM_GF_FRAME, SUB_GF_GAME_SCENE, pData, wDataSize);
		return true;
	}

	bool CTableFrame::PerformStandUpAction(IRoomUserItem * pIServerUserItem, bool bInitiative)
	{
		//效验参数
		if (pIServerUserItem == nullptr)
		{
			return false;
		}

		if (pIServerUserItem->GetTableID() != m_wTableID)
		{
			return false;
		}

		if (pIServerUserItem->GetChairID() > m_wChairCount)
		{
			return false;
		}

		//用户属性
		uint16 wChairID = pIServerUserItem->GetChairID();
		uint8 cbUserStatus = pIServerUserItem->GetUserStatus();
		IRoomUserItem * pITableUserItem = GetTableUserItem(wChairID);

		//游戏用户
		bool bUserStatus = (cbUserStatus == US_PLAYING) || (cbUserStatus == US_OFFLINE);
		if (m_bGameStarted && bUserStatus)
		{
			//结束游戏
			uint8 cbConcludeReason = (cbUserStatus == US_OFFLINE) ? GER_NETWORK_ERROR : GER_USER_LEAVE;
			m_pITableFrameSink->OnEventGameConclude(wChairID, pIServerUserItem, cbConcludeReason);

			//离开算分
			if (m_pGameServiceOption->cbOffLineTrustee == TRUE && m_pITableFrameSink->OnEventGetBetStatus(wChairID, pIServerUserItem))
			{
				pIServerUserItem->SetUserStatus(US_OFFLINE, m_wTableID, wChairID);
				pIServerUserItem->SetClientReady(true);
				return true;
			}

			//离开判断
			if (m_TableUserItemArray[wChairID] != pIServerUserItem) return true;
		}

		//设置变量
		if (pIServerUserItem == pITableUserItem)
		{
			//删除定时
			if (m_pGameServiceOption->wChairCount < MAX_CHAIR) 
			{
				//KillGameTimer(IDI_START_OVERTIME + wChairID);
			}

			//设置变量
			m_TableUserItemArray[wChairID] = nullptr;

			//解除积分锁定
			m_pIMainServiceFrame->UnLockScoreLockUser(pIServerUserItem->GetUserID(), pIServerUserItem->GetInoutIndex(), LER_NORMAL);

			//事件通知
			if (m_pITableUserAction != nullptr)
			{
				m_pITableUserAction->OnActionUserStandUp(wChairID, pIServerUserItem, false);
			}

			//用户状态
			pIServerUserItem->SetClientReady(false);
			pIServerUserItem->SetUserStatus((cbUserStatus == US_OFFLINE) ? US_NULL : US_FREE, INVALID_TABLE, INVALID_CHAIR);

			//变量定义
			bool bTableStarted = IsTableStarted();
			m_wUserCount = GetSitUserCount();

			//结束桌子
			ConcludeTable();

			//开始判断
			if (EfficacyStartGame(INVALID_CHAIR))
			{
				StartGame();
			}

			//发送状态
			if (bTableStarted != IsTableStarted())
			{
				SendTableStatus();
			}
			return true;
		}
		return true;
	}

	bool CTableFrame::PerformSitDownAction(uint16 wChairID, IRoomUserItem * pIServerUserItem, const char * szPassword)
	{
		//效验参数
		assert((pIServerUserItem != nullptr) && (wChairID < m_wChairCount));
		assert((pIServerUserItem->GetTableID() == INVALID_TABLE) && (pIServerUserItem->GetChairID() == INVALID_CHAIR));

		//变量定义
		tagUserInfo * pUserInfo = pIServerUserItem->GetUserInfo();
		IRoomUserItem * pITableUserItem = GetTableUserItem(wChairID);

		//积分变量
		uint64 lUserScore = pIServerUserItem->GetUserScore();
		uint64 lMinTableScore = m_pGameServiceOption->lMinEnterScore;
		//uint64 lLessEnterScore = m_pITableFrameSink->QueryLessEnterScore(wChairID, pIServerUserItem);

		//椅子判断
		if (pITableUserItem != nullptr)
		{
			//构造信息
			//TCHAR szDescribe[128] = TEXT("");
			//_sntprintf(szDescribe, CountArray(szDescribe), TEXT("椅子已经被 [ %s ] 捷足先登了，下次动作要快点了！"), pITableUserItem->GetNickName());
			//SendRequestFailure(pIServerUserItem, szDescribe, REQUEST_FAILURE_NORMAL);
			return false;
		}

		//设置变量
		m_TableUserItemArray[wChairID] = pIServerUserItem;
		//m_wDrawCount = 0;

		//用户状态
		if (IsGameStarted())
		{
			pIServerUserItem->SetClientReady(false);
			pIServerUserItem->SetUserStatus(US_READY, m_wTableID, wChairID);
		}
		else
		{
			////设置变量
			//m_wOffLineCount[wChairID] = 0L;
			//m_dwOffLineTime[wChairID] = 0L;

			////锁定游戏币
			//if (m_pGameServiceOption->lServiceScore > 0L)
			//{
			//	m_lFrozenedScore[wChairID] = m_pGameServiceOption->lServiceScore;
			//	pIServerUserItem->FrozenedUserScore(m_pGameServiceOption->lServiceScore);
			//}
			//设置状态
			pIServerUserItem->SetClientReady(false);
			pIServerUserItem->SetUserStatus(US_PLAYING, m_wTableID, wChairID);
		}

		//设置变量
		m_wUserCount = GetSitUserCount();

		//启动定时
		if (!IsGameStarted())
		{
			//SetGameTimer(IDI_START_OVERTIME + wChairID, TIME_OVERTIME, 1, wChairID);
		}

		//事件通知
		if (m_pITableUserAction != nullptr)
		{
			if (m_wChairCount >= MAX_CHAIR)
			{
				m_pITableUserAction->OnActionUserSitDown(wChairID, pIServerUserItem, false);
			}
			else
			{
				m_pITableUserAction->OnActionUserSitDown(wChairID, pIServerUserItem, false);
			}
		}
		return true;
	}

	bool CTableFrame::OnEventSocketFrame(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem)
	{
		//游戏处理
		if (m_pITableFrameSink->OnFrameMessage(wSubCmdID, pData, wDataSize, pIServerUserItem)) return true;

		//默认处理
		switch (wSubCmdID)
		{
			case SUB_GF_GAME_OPTION:	//游戏配置
			{
				//效验参数
				assert(wDataSize == sizeof(CMD_GF_GameOption));
				if (wDataSize != sizeof(CMD_GF_GameOption)) return false;

				//变量定义
				CMD_GF_GameOption * pGameOption = (CMD_GF_GameOption *)pData;

				//获取属性
				uint16 wChairID = pIServerUserItem->GetChairID();
				uint8 cbUserStatus = pIServerUserItem->GetUserStatus();

				//设置状态
				pIServerUserItem->SetClientReady(true);
				
				//发送状态
				CMD_GF_GameStatus GameStatus;
				GameStatus.cbGameStatus = 0;
				GameStatus.cbAllowLookon = TRUE;
				m_pIMainServiceFrame->SendData(pIServerUserItem, MDM_GF_FRAME, SUB_GF_GAME_STATUS, &GameStatus, sizeof(GameStatus));

				//发送消息
				std::string strMsg = Util::StringFormat(LogonError[LEC_USER_ENTER_TABLE].c_str(), m_pGameServiceOption->strGameName);
				m_pIMainServiceFrame->SendGameMessage(pIServerUserItem, strMsg.c_str(), SMT_CHAT);

				//发送场景
				bool bSendSecret = cbUserStatus != US_LOOKON;
				m_pITableFrameSink->OnEventSendGameScene(wChairID, pIServerUserItem, 0, bSendSecret);

				//准备人数
				for (uint16 i = 0; i < m_wChairCount; ++i)
				{
					//获取用户
					IRoomUserItem * pITableUserItem = GetTableUserItem(i);
					if (pITableUserItem != nullptr) continue;

					//发送状态
					CMD_GF_GameUserData GameUserData;
					GameUserData.cbUserCharID = i;
					m_pIMainServiceFrame->SendData(pIServerUserItem, MDM_GF_FRAME, SUB_GF_USER_DATA, &GameUserData, sizeof(GameUserData));
				}

				//开始判断
				if ((cbUserStatus == US_READY) && (EfficacyStartGame(wChairID)))
				{
					StartGame();
				}
				return true;
			}
		}
		return false;
	}

	bool CTableFrame::EfficacyStartGame(uint16 wReadyChairID)
	{
		return false;
	}

	bool CTableFrame::InitializationFrame(uint16 wTableID, tagTableFrameParameter & TableFrameParameter)
	{
		m_wTableID = wTableID;
		m_wChairCount = TableFrameParameter.pGameServiceOption->wChairCount;
		m_pGameServiceOption = TableFrameParameter.pGameServiceOption;

		m_pITCPSocketService = TableFrameParameter.pITCPSocketService;
		m_pIMainServiceFrame = TableFrameParameter.pIMainServiceFrame;
		
		//创建桌子
		IGameServiceManager * pIGameServiceManager = TableFrameParameter.pIGameServiceManager;
		m_pITableFrameSink = (ITableFrameSink *)pIGameServiceManager->CreateTableFrameSink(IID_ITableFrameSink);

		//扩展接口
		m_pITableUserAction = QUERY_OBJECT_PTR_INTERFACE(m_pITableFrameSink, ITableUserAction);
		return true;
	}

	void CTableFrame::OnGetGameRecord(void * GameRecord)
	{
		m_pITableFrameSink->OnGetGameRecord(GameRecord);
	}

	uint16 CTableFrame::GetNullChairID()
	{
		//椅子搜索
		for (uint16 i = 0; i < m_wChairCount; ++i)
		{
			if (m_TableUserItemArray[i] == nullptr)
			{
				return i;
			}
		}
		return INVALID_CHAIR;
	}

	uint16 CTableFrame::GetSitUserCount()
	{
		//变量定义
		uint16 wUserCount = 0;

		//数目统计
		for (uint16 i = 0; i < m_wChairCount; ++i)
		{
			if (GetTableUserItem(i) != nullptr)
			{
				++wUserCount;
			}
		}
		return wUserCount;
	}

	bool CTableFrame::SendTableStatus()
	{
		return false;
	}
	
}