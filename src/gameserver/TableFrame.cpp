#include "TableFrame.h"

namespace Game
{
	CTableFrame::CTableFrame() : 
		m_wTableID(0),
		m_wChairCount(0),
		m_pITCPSocketService(nullptr),
		m_pGameServiceOption(nullptr)
	{

	}

	CTableFrame::~CTableFrame()
	{

	}

	bool CTableFrame::InitializationFrame(uint16 wTableID, tagTableFrameParameter & TableFrameParameter)
	{
		m_wTableID = wTableID;
		m_wChairCount = TableFrameParameter.pGameServiceOption->wChairCount;
		m_pGameServiceOption = TableFrameParameter.pGameServiceOption;

		m_pITCPSocketService = TableFrameParameter.pITCPSocketService;
		
		//´´½¨×À×Ó
		IGameServiceManager * pIGameServiceManager = TableFrameParameter.pIGameServiceManager;
		m_pITableFrameSink = (ITableFrameSink *)pIGameServiceManager->CreateTableFrameSink(IID_ITableFrameSink);

		return false;
	}
	
}