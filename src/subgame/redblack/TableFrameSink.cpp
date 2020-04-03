#include "TableFrameSink.h"

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
		return false;
	}
	bool CTableFrameSink::OnEventGameStart()
	{
		return false;
	}
	bool CTableFrameSink::OnEventGameConclude(uint16 wChairID, IServerUserItem * pIServerUserItem, uint8 cbReason)
	{
		return false;
	}
	bool CTableFrameSink::OnEventSendGameScene(uint16 wChairID, IServerUserItem * pIServerUserItem, uint8 cbGameStatus, bool bSendSecret)
	{
		return false;
	}
	bool CTableFrameSink::OnGameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IServerUserItem * pIServerUserItem)
	{
		return false;
	}
	bool CTableFrameSink::OnFrameMessage(uint16 wSubCmdID, void * pData, uint16 wDataSize, IServerUserItem * pIServerUserItem)
	{
		return false;
	}
}