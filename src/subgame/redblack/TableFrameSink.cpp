#include "TableFrameSink.h"
#include "CMD_GameServer.h"

namespace SubGame
{
	CTableFrameSink::CTableFrameSink()
	{
	}
	CTableFrameSink::~CTableFrameSink()
	{
	}
	void CTableFrameSink::Release()
	{
	}
	void * CTableFrameSink::QueryInterface(GGUID Guid)
	{
		QUERY_INTERFACE(ITableFrameSink, Guid);
		QUERY_INTERFACE_IUNKNOWNEX(ITableFrameSink, Guid);
		return nullptr;
	}
	void CTableFrameSink::RepositionSink()
	{
	}
	bool CTableFrameSink::Initialization(IUnknownEx * pIUnknownEx)
	{
		//查询接口
		assert(pIUnknownEx != nullptr);
		m_pITableFrame = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, ITableFrame);
		if (m_pITableFrame == nullptr) return false;

		return true;
	}
	bool CTableFrameSink::OnActionUserOffLine(uint16 wChairID, IRoomUserItem * pIServerUserItem)
	{
		return false;
	}
	bool CTableFrameSink::OnActionUserConnect(uint16 wChairID, IRoomUserItem * pIServerUserItem)
	{
		return false;
	}
	bool CTableFrameSink::OnActionUserSitDown(uint16 wChairID, IRoomUserItem * pIServerUserItem, bool bLookonUser)
	{
		return false;
	}
	bool CTableFrameSink::OnActionUserStandUp(uint16 wChairID, IRoomUserItem * pIServerUserItem, bool bLookonUser)
	{
		return false;
	}
	bool CTableFrameSink::OnActionUserOnReady(uint16 wChairID, IRoomUserItem * pIServerUserItem, void * pData, uint16 wDataSize)
	{
		return false;
	}
	bool CTableFrameSink::OnEventGameStart()
	{
		return false;
	}
	bool CTableFrameSink::OnEventGameConclude(uint16 wChairID, IRoomUserItem * pIServerUserItem, uint8 cbReason)
	{
		return false;
	}
	bool CTableFrameSink::OnEventSendGameScene(uint16 wChairID, IRoomUserItem * pIServerUserItem, uint8 cbGameStatus, bool bSendSecret)
	{
		switch (cbGameStatus)
		{
			case 0:		//游戏状态
			{
				//发送场景
				bool bSuccess = m_pITableFrame->SendGameScene(pIServerUserItem, nullptr, 0);
				return true;
			}
		}
		return false;
	}
	bool CTableFrameSink::OnEventGetBetStatus(uint16 wChairID, IRoomUserItem * pIServerUserItem)
	{
		return false;
	}
	void CTableFrameSink::OnGetGameRecord(void * GameRecord)
	{
		CMD_GF_RBRoomStatus *pRoomStatus = (CMD_GF_RBRoomStatus *)GameRecord;

		pRoomStatus->tagGameInfo.wTableID = 1;
		pRoomStatus->tagGameInfo.cbGameStatus = 0;

		pRoomStatus->tagTimeInfo.cbBetTime = 15;
		pRoomStatus->tagTimeInfo.cbEndTime = 15;
		pRoomStatus->tagTimeInfo.cbPassTime = (uint32)time(nullptr) - 0;
		pRoomStatus->tagTimeInfo.lMinXianHong = 5;
		pRoomStatus->tagTimeInfo.lMaxXianHong = 10;

		//客户端只显示48条
		int nIndex = 0;

		pRoomStatus->cbRecordCount = 0;
		int nArrayIndex = 0;

		//while (nIndex != m_nRecordLast)
		//{
		//	if (nArrayIndex >= 48)
		//		break;
		//	pRoomStatus->GameRecordArrary[nArrayIndex].cbAreaWin[0] = m_GameRecordArrary[nIndex].bPlayer;
		//	pRoomStatus->GameRecordArrary[nArrayIndex].cbAreaWin[1] = m_GameRecordArrary[nIndex].bBanker;
		//	pRoomStatus->GameRecordArrary[nArrayIndex].cbAreaWin[2] = m_GameRecordArrary[nIndex].bPing;
		//	pRoomStatus->GameRecordArrary[nArrayIndex].cbAreaWin[3] = m_GameRecordArrary[nIndex].cbCardType;

		//	nArrayIndex++;
		//	nIndex = (nIndex + 1) % MAX_SCORE_HISTORY;
		//}
	}
	bool CTableFrameSink::OnGameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem)
	{
		return false;
	}
	bool CTableFrameSink::OnFrameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IRoomUserItem * pIServerUserItem)
	{
		return false;
	}
}