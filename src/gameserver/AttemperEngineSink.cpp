#include "AttemperEngineSink.h"
#include "Timer.h"
#include "ServiceUnits.h"
#include "CMD_Correspond.h"
#include "CMD_LogonServer.h"
#include "CMD_GameServer.h"
#include "Implementation/LogonDatabase.h"
#include "Log.h"
#include "INIReader.h"
#include "StringUtility.h"
#include "BigNumber.h"

#define MAX_LINK_COUNT 512
#define OPEN_SWITCH		1
#define CLIENT_SWITCH	0

namespace Game
{
	using namespace LogComm;

#define IDI_CONNECT_CORRESPOND		(IDI_MAIN_MODULE_START+3)			//连接时间

#define LOGON_FAILURE(linkid, errorcode) \
	if (OnLogonFailure(linkid, errorcode)) { \
		return true;	\
	}

	CAttemperEngineSink::CAttemperEngineSink():
		m_bNeekCorrespond(true),
		m_pBindParameter(nullptr),
		m_pGameAddressOption(nullptr),
		m_pGameServiceOption(nullptr)
	{
		
	}

	CAttemperEngineSink::~CAttemperEngineSink()
	{
		for (size_t i = 0; i < m_TableFrameArray.size(); ++i)
		{
			PDELETE(m_TableFrameArray[i]);
		}
		m_TableFrameArray.clear();
	}

	void CAttemperEngineSink::Release()
	{
	}

	void * CAttemperEngineSink::QueryInterface(GGUID uuid)
	{
		QUERY_INTERFACE(IMainServiceFrame, uuid);
		QUERY_INTERFACE(IAttemperEngineSink, uuid);
		QUERY_INTERFACE(IRoomUserItemSink, uuid);
		QUERY_INTERFACE_IUNKNOWNEX(IAttemperEngineSink, uuid);
		return nullptr;
	}

	bool CAttemperEngineSink::OnAttemperEngineStart(IUnknownEx * pIUnknownEx)
	{
		m_pBindParameter = new tagBindParameter[MAX_LINK_COUNT];
		memset(m_pBindParameter, 0, sizeof(tagBindParameter)*MAX_LINK_COUNT);

		InitTableFrameArray();

		if (!m_ServerUserManager.SetServerUserItemSink(QUERY_ME_INTERFACE(IRoomUserItemSink)))
		{
			assert(nullptr);
			return false;
		}
		return true;
	}

	bool CAttemperEngineSink::OnAttemperEngineConclude(IUnknownEx * pIUnknownEx)
	{
		m_pITCPSocketService = nullptr;
		return false;
	}

	bool CAttemperEngineSink::OnEventTCPSocketLink(uint16 wServiceID, int iErrorCode)
	{
		//协调连接
		if (wServiceID == NETWORK_CORRESPOND)
		{
			//错误判断
			if (iErrorCode != 0)
			{
				int iConnectTime = sConfigMgr->GetInt32("LocalNet", "ConnectTime", 5);
				LOG_INFO("server.game", "Correspond server connection failed [ %d ], will reconnect in %d seconds", iErrorCode, iConnectTime);
				m_pITimerEngine->SetTimer(IDI_CONNECT_CORRESPOND, iConnectTime * 1000, 1);
				return false;
			}

			//提示消息
			LOG_INFO("server.game", "正在注册游戏登录服务器...");

			//变量定义
			CMD_CS_C_RegisterRoom RegisterRoom;
			memset(&RegisterRoom, 0, sizeof(RegisterRoom));

			//设置变量
			RegisterRoom.wKindID			= m_pGameServiceOption->wKindID;
			RegisterRoom.wServerID			= m_pGameServiceOption->wServerID;
			RegisterRoom.wServerPort		= m_pGameServiceOption->wServerPort;
			RegisterRoom.lCellScore			= m_pGameServiceOption->lCellScore;
			RegisterRoom.lEnterScore		= m_pGameServiceOption->lMinEnterScore;
			RegisterRoom.dwOnLineCount		= m_ServerUserManager.GetUserItemCount();
			RegisterRoom.dwFullCount		= m_pGameServiceOption->wMaxPlayer;
			RegisterRoom.wTableCount		= m_pGameServiceOption->wTableCount;
			RegisterRoom.dwServerRule		= 0;//vCustomRule

			snprintf(RegisterRoom.szServerName, sizeof(RegisterRoom.szServerName), "%s", m_pGameServiceOption->strGameName);
			snprintf(RegisterRoom.szServerAddr, sizeof(RegisterRoom.szServerAddr), "%s", m_pGameAddressOption->szIP);

			//发送数据
			m_pITCPSocketService->SendData(MDM_CS_REGISTER, SUB_CS_C_REGISTER_ROOM, &RegisterRoom, sizeof(RegisterRoom));

			return true;
		}

		return true;
	}

	bool CAttemperEngineSink::OnEventTCPSocketShut(uint16 wServiceID, uint8 cbShutReason)
	{
		//协调连接
		if (wServiceID == NETWORK_CORRESPOND)
		{
			//重连判断
			if (m_bNeekCorrespond)
			{
				//构造提示
				int iConnectTime = sConfigMgr->GetInt32("LocalNet", "ConnectTime", 5);
				LOG_INFO("server.game", "The connection to the correspond server is closed and will reconnect in %d seconds", iConnectTime);

				//设置时间
				assert(m_pITimerEngine != nullptr);
				m_pITimerEngine->SetTimer(IDI_CONNECT_CORRESPOND, iConnectTime * 1000, 1);
			}
			return true;
		}

		return false;
	}

	bool CAttemperEngineSink::OnEventTCPSocketRead(uint16 wServiceID, TCP_Command Command, void * pData, uint16 wDataSize)
	{
		//协调连接
		if (wServiceID == NETWORK_CORRESPOND)
		{
			switch (Command.wMainCmdID)
			{
				case MDM_CS_REGISTER:		//注册服务
				{
					return OnTCPSocketMainRegister(Command.wSubCmdID, pData, wDataSize);
				}
				case MDM_CS_ROOM_INFO:	//服务信息
				{
					return OnTCPSocketMainServiceInfo(Command.wSubCmdID, pData, wDataSize);
				}
			}
		}
		return false;
	}

	bool CAttemperEngineSink::OnEventTCPNetworkBind(uint32 dwClientAddr, uint32 dwSocketID)
	{
		//获取索引
		assert(dwSocketID < MAX_LINK_COUNT);
		if (dwSocketID >= MAX_LINK_COUNT) return false;

		//变量定义
		tagBindParameter * pBindParameter = (m_pBindParameter + dwSocketID);

		//设置变量
		pBindParameter->dwSocketID = dwSocketID;
		pBindParameter->dwClientAddr = dwClientAddr;
		return true;
	}

	bool CAttemperEngineSink::OnEventTCPNetworkShut(uint32 dwClientAddr, uint32 dwSocketID)
	{
		//变量定义
		uint16 wBindIndex = LOWORD(dwSocketID);
		tagBindParameter * pBindParameter = GetBindParameter(wBindIndex);
		if (pBindParameter == nullptr) return false;

		//获取用户
		IRoomUserItem * pIServerUserItem = pBindParameter->pIServerUserItem;
		uint16 wTableID = INVALID_WORD;

		try
		{
			//用户处理
			if (pIServerUserItem != nullptr)
			{
				//变量定义
				wTableID = pIServerUserItem->GetTableID();

				//断线处理
				if (wTableID != INVALID_TABLE)
				{
					//解除绑定
					pIServerUserItem->DetachBindStatus();
					if (wTableID < m_pGameServiceOption->wTableCount)
					{
						//断线通知
						assert(wTableID < m_pGameServiceOption->wTableCount);
						//m_TableFrameArray[wTableID]->OnEventUserOffLine(pIServerUserItem);
					}
					else //先不处理看有什么问题
					{

					}
				}
				else
				{
					pIServerUserItem->SetUserStatus(US_NULL, INVALID_TABLE, INVALID_CHAIR);
				}
			}
		}
		catch (...)
		{
			LOG_INFO("server.game", "关闭连接异常: wTableID=%d", wTableID);
		}

		//清除信息
		memset((m_pBindParameter + dwSocketID), 0, sizeof(tagBindParameter));
		return false;
	}

	bool CAttemperEngineSink::OnEventTCPNetworkRead(Net::TCP_Command Command, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		switch (Command.wMainCmdID)
		{
			case MDM_GR_LOGON:			//登录命令
			{
				return OnTCPNetworkMainLogon(Command.wSubCmdID, pData, wDataSize, dwSocketID);
			}
			case MDM_GR_USER:
			{
				return OnTCPNetworkMainUser(Command.wSubCmdID, pData, wDataSize, dwSocketID);
			}
			case MDM_GF_FRAME:
			{
				return OnTCPNetworkMainFrame(Command.wSubCmdID, pData, wDataSize, dwSocketID);
			}
		}
		return false;
	}

	bool CAttemperEngineSink::OnEventControl(uint16 wControlID, void * pData, uint16 wDataSize)
	{
		switch (wControlID)
		{
			case SUC_LOAD_DB_GAME_LIST:
			{
				//事件通知
				ControlResult ControlResult;
				ControlResult.cbSuccess = 1;
				SrvUnitsMgr->PostControlRequest(UDC_LOAD_DB_LIST_RESULT, &ControlResult, sizeof(ControlResult));
				return true;
			}
			case SUC_CONNECT_CORRESPOND:
			{
				//发起连接
				m_pITCPSocketService->Connect(sConfigMgr->Get("CorrespondNet", "BindIP", "127.0.0.1"), sConfigMgr->GetInt32("CorrespondNet", "Port", 8600));
				return true;
			}
		}
		return false;
	}

	bool CAttemperEngineSink::OnEventTimer(uint32 dwTimerID)
	{
		//时间处理
		try 
		{
			//桌子时间
			if ((dwTimerID >= IDI_TABLE_MODULE_START) && (dwTimerID <= IDI_TABLE_MODULE_FINISH))
			{
				//桌子号码
				uint32 dwTableTimerID = dwTimerID - IDI_TABLE_MODULE_START;
				uint16 wTableID = (uint16)(dwTableTimerID / IDI_TABLE_MODULE_RANGE);

				//时间效验
				if (wTableID >= (uint16)m_TableFrameArray.size())
				{
					assert(nullptr);
					return false;
				}

				//时间通知
				CTableFrame * pTableFrame = m_TableFrameArray[wTableID];
				return pTableFrame->OnEventTimer(dwTableTimerID%IDI_TABLE_MODULE_RANGE);
			}

			if (IDI_CONNECT_CORRESPOND)
			{
				std::string strCorrespondAddress = sConfigMgr->Get("CorrespondNet", "BindIP", "127.0.0.1");
				uint16 wCorrespondPort = sConfigMgr->GetInt32("CorrespondNet", "Port", 8600);
				m_pITCPSocketService->Connect(strCorrespondAddress, wCorrespondPort);
				LOG_INFO("server.game", "Connecting to the correspond server [ %s:%d ]", strCorrespondAddress, wCorrespondPort);
				return true;
			}
		}
		catch (...)
		{

		}
		return false;
	}

	bool CAttemperEngineSink::SendRoomMessage(char * lpszMessage, uint16 wType)
	{
		return false;
	}

	bool CAttemperEngineSink::SendGameMessage(char * lpszMessage, uint16 wType)
	{
		return false;
	}

	bool CAttemperEngineSink::SendRoomMessage(IRoomUserItem * pIServerUserItem, const char * lpszMessage, uint16 wType)
	{
		//效验参数
		assert(pIServerUserItem != NULL);
		if (pIServerUserItem == NULL) return false;

		//发送数据
		if (pIServerUserItem->GetBindIndex() != INVALID_WORD)
		{
			//变量定义
			CMD_CM_SystemMessage SystemMessage;
			memset(&SystemMessage, 0, sizeof(SystemMessage));

			//构造数据
			SystemMessage.wType = wType;
			SystemMessage.wLength = strlen(lpszMessage) + 1;

			std::wstring wstrMessage = Util::StringUtility::StringToWString(lpszMessage);
			swprintf(SystemMessage.szString, sizeof(SystemMessage.szString), L"%s", wstrMessage.c_str());

			//变量定义
			uint32 dwUserIndex = pIServerUserItem->GetBindIndex();
			tagBindParameter * pBindParameter = GetBindParameter(dwUserIndex);

			//数据属性
			uint16 wHeadSize = sizeof(SystemMessage) - sizeof(SystemMessage.szString);
			uint16 wSendSize = wHeadSize + SystemMessage.wLength;

			//常规用户
			m_pITCPNetworkEngine->SendData(pBindParameter->dwSocketID, MDM_CM_SYSTEM, SUB_CM_SYSTEM_MESSAGE, &SystemMessage, wSendSize);
			return true;
		}

		return false;
	}

	bool CAttemperEngineSink::SendGameMessage(IRoomUserItem * pIServerUserItem, const char * lpszMessage, uint16 wType)
	{
		return false;
	}

	bool CAttemperEngineSink::SendRoomMessage(uint32 dwSocketID, const char * lpszMessage, uint16 wType, bool bAndroid)
	{
		return false;
	}

	bool CAttemperEngineSink::SendData(uint32 dwSocketID, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		//网络用户
		m_pITCPNetworkEngine->SendData(dwSocketID, wMainCmdID, wSubCmdID, pData, wDataSize);
		return true;
	}

	bool CAttemperEngineSink::SendData(IRoomUserItem * pIServerUserItem, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		//效验参数
		assert(pIServerUserItem != nullptr);
		if (pIServerUserItem == nullptr) return false;

		//常规用户
		uint16 wBindIndex = pIServerUserItem->GetBindIndex();
		tagBindParameter * pBindParameter = GetBindParameter(wBindIndex);
		m_pITCPNetworkEngine->SendData(pBindParameter->dwSocketID, wMainCmdID, wSubCmdID, pData, wDataSize);
		return true;
	}

	bool CAttemperEngineSink::SendDataBatch(uint16 wCmdTable, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		CRoomUserManager::CRoomUserItemMap ruim = m_ServerUserManager.TraverseRoomUserList();
		for (CRoomUserManager::RUIM_IT it = ruim.begin(); it != ruim.end(); ++it)
		{
			IRoomUserItem *pITargetUserItem = it->second;
			uint16 wTagerTableID = pITargetUserItem->GetTableID();

			//状态过滤
			if ((pITargetUserItem->GetUserStatus() >= US_SIT) && pITargetUserItem->IsClientReady())
			{
				if (wCmdTable != INVALID_WORD && wTagerTableID != wCmdTable)continue;
			}

			//发送消息
			SendData(pITargetUserItem, wMainCmdID, wSubCmdID, pData, wDataSize);
		}

		return false;
	}

	void CAttemperEngineSink::UnLockScoreLockUser(uint32 dwUserID, uint32 dwInoutIndex, uint32 dwLeaveReason)
	{
		PerformUnlockScore(dwUserID, dwInoutIndex, dwLeaveReason);
	}

	bool CAttemperEngineSink::OnEventUserItemStatus(IRoomUserItem * pIServerUserItem, uint16 wOldTableID, uint16 wOldChairID)
	{
		//效验参数
		assert(pIServerUserItem != nullptr);
		if (pIServerUserItem == nullptr) return false;
		
		//变量定义
		CMD_GR_UserStatus UserStatus;
		memset(&UserStatus, 0, sizeof(UserStatus));

		//构造数据
		UserStatus.dwUserID = pIServerUserItem->GetUserID();
		UserStatus.UserStatus.wTableID = pIServerUserItem->GetTableID();
		UserStatus.UserStatus.wChairID = pIServerUserItem->GetChairID();
		UserStatus.UserStatus.cbUserStatus = pIServerUserItem->GetUserStatus();

		//用户信息
		uint16 wTableID = pIServerUserItem->GetTableID();
		uint8 cbUserStatus = pIServerUserItem->GetUserStatus();

	
		if (cbUserStatus >= US_SIT)
		{
			if (wOldTableID == INVALID_TABLE && cbUserStatus == US_SIT)
			{
			}
			else if (m_pGameServiceOption->wChairCount >= MAX_CHAIR)
			{
				SendDataBatch(wOldTableID, MDM_GR_USER, SUB_GR_USER_STATUS, &UserStatus, sizeof(UserStatus));
			}
			SendDataBatch(wTableID, MDM_GR_USER, SUB_GR_USER_STATUS, &UserStatus, sizeof(UserStatus));
		}
		else
		{
			SendDataBatch(wOldTableID, MDM_GR_USER, SUB_GR_USER_STATUS, &UserStatus, sizeof(UserStatus));
		}

		//发送玩家状态
		CMD_CS_C_UserStatus  Status;
		Status.dwUserID = UserStatus.dwUserID;
		Status.cbUserStatus = UserStatus.UserStatus.cbUserStatus;
		Status.wKindID = m_pGameServiceOption->wKindID;
		Status.wServerID = m_pGameServiceOption->wServerID;
		Status.wTableID = UserStatus.UserStatus.wTableID;
		Status.wChairID = UserStatus.UserStatus.wChairID;
		m_pITCPSocketService->SendData(MDM_CS_USER_COLLECT, SUB_CS_C_USER_STATUS, &Status, sizeof(Status));

		//离开判断
		if (pIServerUserItem->GetUserStatus() == US_NULL)
		{
			//获取绑定
			uint16 wBindIndex = pIServerUserItem->GetBindIndex();
			tagBindParameter * pBindParameter = GetBindParameter(wBindIndex);

			//绑带处理
			if (pBindParameter != nullptr)
			{
				//绑定处理
				if (pBindParameter->pIServerUserItem == pIServerUserItem)
				{
					pBindParameter->pIServerUserItem = nullptr;
				}

				//中断网络
				if (pBindParameter->dwSocketID != 0)
				{
					m_pITCPNetworkEngine->ShutDownSocket(pBindParameter->dwSocketID);
				}
			}

			//离开处理
			OnEventUserLogout(pIServerUserItem, 0);
		}
		return true;
	}

	bool CAttemperEngineSink::SendUserInfoPacket(IRoomUserItem * pIServerUserItem, uint32 dwSocketID)
	{
		//效验参数
		assert(pIServerUserItem != nullptr);
		if (pIServerUserItem == nullptr) return false;

		//变量定义
		uint8 cbBuffer[SOCKET_TCP_PACKET] = {};
		tagUserInfo * pUserInfo = pIServerUserItem->GetUserInfo();
		
		CMD_GR_MobileUserInfoHead *pUserInfoHead = (CMD_GR_MobileUserInfoHead *)cbBuffer;

		//用户属性
		pUserInfoHead->wFaceID = pUserInfo->wFaceID;
		pUserInfoHead->dwGameID = pUserInfo->dwGameID;
		pUserInfoHead->dwUserID = pUserInfo->dwUserID;

		//用户属性	
		pUserInfoHead->cbGender = pUserInfo->cbGender;

		//用户状态
		pUserInfoHead->wTableID = pUserInfo->wTableID;
		pUserInfoHead->wChairID = pUserInfo->wChairID;
		pUserInfoHead->cbUserStatus = pUserInfo->cbUserStatus;

		//用户局数
		pUserInfoHead->dwWinCount = pUserInfo->dwWinCount;
		pUserInfoHead->dwLostCount = pUserInfo->dwLostCount;
		pUserInfoHead->dwDrawCount = pUserInfo->dwDrawCount;
		pUserInfoHead->dwFleeCount = pUserInfo->dwFleeCount;

		//用户成绩
		pUserInfoHead->lScore = pUserInfo->lScore;

		std::wstring wstrNickName = Util::StringUtility::StringToWString(pUserInfo->szNickName);
		swprintf(pUserInfoHead->szNickName, sizeof(pUserInfoHead->szNickName), L"%s", wstrNickName.c_str());

		if (dwSocketID == INVALID_DWORD)
		{
			SendUserInfoPacketBatch(pIServerUserItem, INVALID_DWORD);
		}
		else
		{
			uint16 wHeadSize = sizeof(CMD_GR_MobileUserInfoHead);
			SendData(dwSocketID, MDM_GR_USER, SUB_GR_USER_ENTER, cbBuffer, wHeadSize);
		}
		return true;
	}

	bool CAttemperEngineSink::SendUserInfoPacketBatch(IRoomUserItem * pIServerUserItem, uint32 dwSocketID)
	{
		//效验参数
		assert(pIServerUserItem != NULL);
		if (pIServerUserItem == NULL) return false;

		//变量定义
		uint8 cbBuffer[SOCKET_TCP_PACKET] = {};
		tagUserInfo * pUserInfo = pIServerUserItem->GetUserInfo();
		CMD_GR_MobileUserInfoHead * pUserInfoHead = (CMD_GR_MobileUserInfoHead *)cbBuffer;

		//用户属性
		pUserInfoHead->wFaceID = pUserInfo->wFaceID;
		pUserInfoHead->dwGameID = pUserInfo->dwGameID;
		pUserInfoHead->dwUserID = pUserInfo->dwUserID;

		//用户属性
		pUserInfoHead->cbGender = pUserInfo->cbGender;

		//用户状态
		pUserInfoHead->wTableID = pUserInfo->wTableID;
		pUserInfoHead->wChairID = pUserInfo->wChairID;
		pUserInfoHead->cbUserStatus = pUserInfo->cbUserStatus;

		//用户局数
		pUserInfoHead->dwWinCount = pUserInfo->dwWinCount;
		pUserInfoHead->dwLostCount = pUserInfo->dwLostCount;
		pUserInfoHead->dwDrawCount = pUserInfo->dwDrawCount;
		pUserInfoHead->dwFleeCount = pUserInfo->dwFleeCount;

		//用户成绩
		pUserInfoHead->lScore = pUserInfo->lScore;

		std::wstring wstrNickName = Util::StringUtility::StringToWString(pUserInfo->szNickName);
		swprintf(pUserInfoHead->szNickName, sizeof(pUserInfoHead->szNickName), L"%s", wstrNickName.c_str());
	
		//发送数据
		uint16 wHeadSize = sizeof(CMD_GR_MobileUserInfoHead);
		if (dwSocketID == INVALID_DWORD)
		{
			SendDataBatch(pUserInfo->wTableID, MDM_GR_USER, SUB_GR_USER_ENTER, cbBuffer, wHeadSize);
		}
		else
		{
			SendData(dwSocketID, MDM_GR_USER, SUB_GR_USER_ENTER, cbBuffer, wHeadSize);
		}

		return true;
	}

	bool CAttemperEngineSink::InitTableFrameArray()
	{
		tagTableFrameParameter TableFrameParameter;
		memset(&TableFrameParameter, 0, sizeof(TableFrameParameter));

		//服务组件
		TableFrameParameter.pIMainServiceFrame = this;
		TableFrameParameter.pITimerEngine = m_pITimerEngine;
		TableFrameParameter.pIGameServiceManager = m_pIGameServiceManager;

		//配置参数
		TableFrameParameter.pGameServiceOption = m_pGameServiceOption;

		//桌子容器
		m_TableFrameArray.resize(m_pGameServiceOption->wTableCount);

		//创建桌子
		for (uint16 i = 0; i < m_pGameServiceOption->wTableCount; ++i)
		{
			//创建对象
			m_TableFrameArray[i] = new CTableFrame;

			//配置桌子
			if (!m_TableFrameArray[i]->InitializationFrame(i, TableFrameParameter))
			{
				return false;
			}
		}
		return true;
	}

	bool CAttemperEngineSink::OnTCPSocketMainRegister(uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		switch (wSubCmdID)
		{
			case SUB_CS_S_REGISTER_FAILURE:		//注册失败
			{
				//变量定义
				CMD_CS_S_RegisterFailure * pRegisterFailure = (CMD_CS_S_RegisterFailure *)pData;

				//效验参数
				assert(wDataSize >= (sizeof(CMD_CS_S_RegisterFailure) - sizeof(pRegisterFailure->szDescribeString)));
				if (wDataSize < (sizeof(CMD_CS_S_RegisterFailure) - sizeof(pRegisterFailure->szDescribeString))) return false;

				//关闭处理
				m_bNeekCorrespond = false;
				m_pITCPSocketService->CloseSocket();

				//显示消息
				LOG_INFO("server.game", "%s", pRegisterFailure->szDescribeString);

				//事件通知
				ControlResult ControlResult;
				ControlResult.cbSuccess = 0;
				SrvUnitsMgr->PostControlRequest(UDC_CORRESPOND_RESULT, &ControlResult, sizeof(ControlResult));
				return true;
			}
		}

		return true;
	}

	bool CAttemperEngineSink::OnTCPSocketMainServiceInfo(uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		switch (wSubCmdID)
		{
			case SUB_CS_S_ROOM_INFO:		//房间信息
			{
				return true;
			}
			case SUB_CS_S_ROOM_ONLINE:	//房间人数
			{
				//效验参数
				assert(wDataSize == sizeof(CMD_CS_S_RoomOnLine));
				if (wDataSize != sizeof(CMD_CS_S_RoomOnLine)) return false;

				//变量定义
				CMD_CS_S_RoomOnLine * pServerOnLine = (CMD_CS_S_RoomOnLine *)pData;
				return true;
			}
			case SUB_CS_S_ROOM_INSERT:	//房间插入
			{
				return true;
			}
			case SUB_CS_S_ROOM_FINISH:	//房间完成
			{
				//事件处理
				ControlResult ControlResult;
				ControlResult.cbSuccess = 1;
				SrvUnitsMgr->PostControlRequest(UDC_CORRESPOND_RESULT, &ControlResult, sizeof(ControlResult));
				return true;
			}
		}
		return false;
	}

	bool CAttemperEngineSink::OnTCPNetworkMainUser(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		switch (wSubCmdID)
		{
			case SUB_GR_USER_SITDOWN:		//用户坐下
			{
				return OnTCPNetworkSubUserSitDown(pData, wDataSize, dwSocketID);
			}
			case SUB_GR_USER_STANDUP:		//用户起立
			{
				return OnTCPNetworkSubUserStandUp(pData, wDataSize, dwSocketID);
			}
		}
		return false;
	}

	bool CAttemperEngineSink::OnTCPNetworkMainLogon(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		switch (wSubCmdID)
		{
			case SUB_GR_LOGON_MOBILE:      //游客登录
			{
				return OnTCPNetworkSubMBLogonVisitor(pData, wDataSize, dwSocketID);
			}
		}
		return false;
	}
	bool CAttemperEngineSink::OnTCPNetworkMainFrame(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		//获取信息
		bool bResult = false;
		uint16 wBindIndex = LOWORD(dwSocketID);
		IRoomUserItem * pIServerUserItem = GetBindUserItem(wBindIndex);

		//用户效验
		assert(pIServerUserItem != nullptr);
		if (pIServerUserItem == nullptr) return false;

		//处理过虑
		uint16 wTableID = pIServerUserItem->GetTableID();
		uint16 wChairID = pIServerUserItem->GetChairID();

		if (wSubCmdID == SUB_GF_BJL_GAME_STATUS)
		{
			bResult = QuerryGameRoomRecordTime(pIServerUserItem);
		}
		else
		{
			if (wTableID != INVALID_TABLE)
			{
				//消息处理 
				CTableFrame * pTableFrame = m_TableFrameArray[wTableID];
				bResult = pTableFrame->OnEventSocketFrame(wSubCmdID, pData, wDataSize, pIServerUserItem);
			}
		}

		if (!bResult)
		{
			LOG_INFO("server.game", "MDM_GF_FRAME 框架命令返回 false");
		}
		return bResult;
	}
	bool CAttemperEngineSink::OnTCPNetworkSubMBLogonVisitor(void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		//效验参数
		assert(wDataSize >= sizeof(CMD_GR_LogonMobile));
		if (wDataSize < sizeof(CMD_GR_LogonMobile))return false;

		//变量定义
		uint16 wBindIndex = LOWORD(dwSocketID);
		IRoomUserItem	 * pIBindUserItem = GetBindUserItem(wBindIndex);
		tagBindParameter * pBindParameter = GetBindParameter(wBindIndex);
		
		//重复判断
		if (pBindParameter == nullptr || pIBindUserItem)
		{
			assert(nullptr);
			return false;
		}

		//处理消息
		CMD_GR_LogonMobile * pLogonMobile = (CMD_GR_LogonMobile *)pData;
		if (pBindParameter->dwClientAddr != 0 && pLogonMobile->szPassword[0] == L'0')
		{
			//发送失败
			SendLogonFailure(LogonError[LEC_PW_EMPTY].c_str(), LEC_PW_EMPTY, dwSocketID);
			return true;
		}

		//设置连接
		pBindParameter->cbClientKind = LinkType::LT_MOBILE;

		//断线重连
		std::string strLogonPass = Util::StringUtility::WStringToString(pLogonMobile->szPassword);
		std::string strMachineID = Util::StringUtility::WStringToString(pLogonMobile->szMachineID);

		IRoomUserItem * pIServerUserItem = m_ServerUserManager.SearchUserItem(pLogonMobile->dwUserID);
		if ((pIServerUserItem != NULL) && (pIServerUserItem->ContrastLogonPass(strLogonPass.c_str()) == true))
		{
			SwitchUserItemConnect(pIServerUserItem, strMachineID.c_str(), wBindIndex);
			return true;
		}

		///////////////////////////////DB查询//////////////////////////////////
		//优化TODO...																			
		//1、后期改成多开登录长连接，所有消息从登录收发，这样就不用再次查询DB二次登录。			
		//2、另外下面其实很多不重要信息可以记录redis里，找个服务不繁忙时间up下数据库。	
		///////////////////////////////////////////////////////////////////////
		
		//查询用户信息
		PreparedStatement *stmt = LogonDatabasePool.GetPreparedStatement(LOGON_SEL_VISITOR_ACCOUNT);
		stmt->SetString(0, "");
		PreparedQueryResult result = LogonDatabasePool.Query(stmt);
		
		if (!result)
		{
			//SendLogonFailure();
			m_pITCPNetworkEngine->ShutDownSocket(dwSocketID);
			return true;
		}

		//获取游戏信息
		Field* field = result->Fetch();
		int id = field[0].GetInt32();
		std::string account = field[1].GetString();
		std::string username = field[2].GetString();
		std::string sha_pass_hash = field[3].GetString();
		std::string face_url = field[4].GetString();
		int limit = field[5].GetInt8();
		uint64 score = field[6].GetUInt64();

		////废弃判断
		//if ((pBindParameter->pIServerUserItem != nullptr) || (pBindParameter->dwSocketID != dwSocketID))
		//{
		//	PerformUnlockScore(pLogonMobile->dwUserID, 0/*pLogonMobile->dwInoutIndex*/, LER_NORMAL);
		//	return true;
		//}

		//最低分数
		if ((m_pGameServiceOption->lMinEnterScore != 0) && (score < m_pGameServiceOption->lMinEnterScore))
		{
			SendLogonFailure(LogonError[LEC_ROOM_ENTER_SCORE_LESS].c_str(), LEC_ROOM_ENTER_SCORE_LESS, pBindParameter->dwSocketID);
			PerformUnlockScore(pLogonMobile->dwUserID, 0/* pDBOLogonSuccess->dwInoutIndex*/, LER_SERVER_CONDITIONS);
			return true;
		}

		//满人判断
		uint16 wMaxPlayer = m_pGameServiceOption->wMaxPlayer;
		uint32 dwOnlineCount = m_ServerUserManager.GetUserItemCount();
		if (dwOnlineCount > (uint32)(wMaxPlayer))
		{
			SendLogonFailure(LogonError[LEC_ROOM_FULL].c_str(), LEC_ROOM_FULL, pBindParameter->dwSocketID);
			PerformUnlockScore(pLogonMobile->dwUserID, 0/* pDBOLogonSuccess->dwInoutIndex*/, LER_SERVER_FULL);
			return true;
		}

		//////////////////////////////////用户变量
		std::string strAnsiNickName;
		Util::StringUtility::Utf8ToConsole(username, strAnsiNickName);

		std::string strAnsiPW;
		Util::StringUtility::Utf8ToConsole(sha_pass_hash, strAnsiPW);

		tagUserInfo UserInfo;
		tagUserInfoPlus UserInfoPlus;
		memset(&UserInfo, 0, sizeof(UserInfo));
		memset(&UserInfoPlus, 0, sizeof(UserInfoPlus));

		//属性资料
		UserInfo.wFaceID = 0;
		UserInfo.dwUserID = pLogonMobile->dwUserID;
		UserInfo.dwGameID = pLogonMobile->dwUserID;
		snprintf(UserInfo.szNickName, sizeof(UserInfo.szNickName), "%s", strAnsiNickName.c_str());

		//用户资料
		UserInfo.cbGender = 0;

		//状态设置
		UserInfo.cbUserStatus = US_FREE;
		UserInfo.wTableID = INVALID_TABLE;
		UserInfo.wChairID = INVALID_CHAIR;

		//积分信息
		UserInfo.lScore = 100;

		//登录信息
		UserInfoPlus.dwLogonTime = (uint32)time(nullptr);
		UserInfoPlus.dwInoutIndex = 0;

		//用户权限
		UserInfoPlus.dwUserRight = 0;

		//辅助变量
		UserInfoPlus.lLimitScore = 10;
		snprintf(UserInfoPlus.szPassword, sizeof(UserInfoPlus.szPassword), "%s", sha_pass_hash.c_str());

		//连接信息
		UserInfoPlus.wBindIndex = wBindIndex;
		UserInfoPlus.dwClientAddr = pBindParameter->dwClientAddr;
		snprintf(UserInfoPlus.szMachineID, sizeof(UserInfoPlus.szMachineID), "%s", strMachineID.c_str());

		//激活用户
		m_ServerUserManager.InsertUserItem(&pIServerUserItem, UserInfo, UserInfoPlus);

		//错误判断
		if (pIServerUserItem == nullptr)
		{
			assert(nullptr);
			PerformUnlockScore(pLogonMobile->dwUserID, 0/*pDBOLogonSuccess->dwInoutIndex*/, LER_SERVER_FULL);
			m_pITCPNetworkEngine->ShutDownSocket(dwSocketID);
			return true;
		}

		//设置用户
		pBindParameter->pIServerUserItem = pIServerUserItem;

		//登录事件
		OnEventUserLogon(pIServerUserItem, false);

		//汇总用户
		//if (m_bCollectUser == true)
		{
			//变量定义
			CMD_CS_C_UserEnter UserEnter;
			memset(&UserEnter, 0, sizeof(UserEnter));

			//设置变量
			UserEnter.dwUserID = pIServerUserItem->GetUserID();
			UserEnter.dwGameID = pIServerUserItem->GetGameID();
			snprintf(UserEnter.szNickName, sizeof(UserEnter.szNickName), "%s", pIServerUserItem->GetNickName());

			//用户详细信息
			tagUserInfo* pUserInfo = pIServerUserItem->GetUserInfo();
			if (pUserInfo) memcpy(&UserEnter.userInfo, pUserInfo, sizeof(tagUserInfo));

			//发送消息
			assert(m_pITCPSocketService);
			m_pITCPSocketService->SendData(MDM_CS_USER_COLLECT, SUB_CS_C_USER_ENTER, &UserEnter, sizeof(UserEnter));
		}
		return true;
	}

	bool CAttemperEngineSink::OnTCPNetworkSubUserSitDown(void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		//效验参数
		assert(wDataSize == sizeof(CMD_GR_UserSitDown));
		if (wDataSize != sizeof(CMD_GR_UserSitDown)) return false;

		//效验数据
		CMD_GR_UserSitDown * pUserSitDown = (CMD_GR_UserSitDown *)pData;

		//获取用户
		uint16 wBindIndex = LOWORD(dwSocketID);
		IRoomUserItem * pIServerUserItem = GetBindUserItem(wBindIndex);

		//用户效验
		assert(pIServerUserItem != nullptr);
		if (pIServerUserItem == nullptr) return false;

		//消息处理
		uint16 wTableID = pIServerUserItem->GetTableID();
		uint16 wChairID = pIServerUserItem->GetChairID();
		uint8 cbUserStatus = pIServerUserItem->GetUserStatus();

		//重复判断
		if ((pUserSitDown->wTableID < m_pGameServiceOption->wTableCount) && (pUserSitDown->wChairID < m_pGameServiceOption->wChairCount))
		{
			CTableFrame * pTableFrame = m_TableFrameArray[pUserSitDown->wTableID];
			if (pTableFrame->GetTableUserItem(pUserSitDown->wChairID) == pIServerUserItem) return true;
		}

		//用户判断
		if (cbUserStatus == US_PLAYING)
		{
			SendUserFailure(pIServerUserItem, LogonError[LEC_USER_PLAYING].c_str());
			return true;
		}

		//离开处理
		if (wTableID != INVALID_TABLE)
		{
			CTableFrame * pTableFrame = m_TableFrameArray[wTableID];
			if (pTableFrame->PerformStandUpAction(pIServerUserItem, true) == false) return true;
		}

		//请求调整
		uint16 wRequestTableID = pUserSitDown->wTableID;
		uint16 wRequestChairID = pUserSitDown->wChairID;
		uint16 wTailChairID = INVALID_CHAIR;

		//寻找位置
		for (uint16 i = 0; i < m_TableFrameArray.size(); ++i)
		{
			//获取空位
			uint16 wNullChairID = m_TableFrameArray[i]->GetNullChairID();

			//调整结果
			if (wNullChairID != INVALID_CHAIR)
			{
				//设置变量
				wRequestTableID = i;
				wRequestChairID = wNullChairID;
				break;
			}
		}

		//结果判断
		if ((wRequestTableID == INVALID_CHAIR) || (wRequestChairID == INVALID_CHAIR))
		{
			SendUserFailure(pIServerUserItem, LogonError[LEC_USER_ROOM_FULL].c_str());
			return true;
		}
		

		//椅子调整
		if (wRequestChairID >= m_pGameServiceOption->wChairCount)
		{
			//效验参数
			assert(wRequestTableID < m_TableFrameArray.size());
			if (wRequestTableID >= m_TableFrameArray.size()) return false;

			//查找空位
			wRequestChairID = m_TableFrameArray[wRequestTableID]->GetNullChairID();

			//结果判断
			if (wRequestChairID == INVALID_CHAIR)
			{
				SendUserFailure(pIServerUserItem, LogonError[LEC_USER_TABLE_NOT_CHAIR].c_str());
				return true;
			}
		}

		//坐下处理
		std::string strPWD = Util::StringUtility::WStringToString(pUserSitDown->szPassword);
		CTableFrame * pTableFrame = m_TableFrameArray[wRequestTableID];
		pTableFrame->PerformSitDownAction(wRequestChairID, pIServerUserItem, strPWD.c_str());
		return true;
	}

	bool CAttemperEngineSink::OnTCPNetworkSubUserStandUp(void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		//效验参数
		CMD_GR_UserStandUp * pUserStandUp = (CMD_GR_UserStandUp *)pData;
		if (nullptr == pUserStandUp || wDataSize != sizeof(CMD_GR_UserStandUp))
		{
			assert(false);
			return false;
		}

		//获取用户
		uint16 wBindIndex = LOWORD(dwSocketID);
		IRoomUserItem * pIServerUserItem = GetBindUserItem(wBindIndex);

		//用户效验
		if (pIServerUserItem == NULL)
		{
			assert(false);
			return false;
		}
	
		//效验数据
		if (pUserStandUp->wChairID >= m_pGameServiceOption->wChairCount) return false;
		if (pUserStandUp->wTableID >= (uint16)m_TableFrameArray.size()) return false;

		//消息处理
		uint16 wTableID = pIServerUserItem->GetTableID();
		uint16 wChairID = pIServerUserItem->GetChairID();
		if ((wTableID != pUserStandUp->wTableID) || (wChairID != pUserStandUp->wChairID)) return true;

		//用户判断
		if ((pUserStandUp->cbForceLeave == 0) && (pIServerUserItem->GetUserStatus() == US_PLAYING))
		{
			SendRoomMessage(pIServerUserItem, "您正在游戏中，暂时不能离开，请先结束当前游戏！", SMT_CHAT | SMT_EJECT | SMT_GLOBAL);
			return true;
		}

		//离开处理
		if (wTableID != INVALID_TABLE)
		{
			auto pTableFrame = m_TableFrameArray[wTableID];
			if (!pTableFrame->PerformStandUpAction(pIServerUserItem)) return true;
		}
		return true;
	}

	bool CAttemperEngineSink::SwitchUserItemConnect(IRoomUserItem * pIServerUserItem, const char szMachineID[LEN_MACHINE_ID], uint16 wTargetIndex)
	{
		//效验参数
		assert((pIServerUserItem != nullptr) && (wTargetIndex != INVALID_WORD));
		if ((pIServerUserItem == nullptr) || (wTargetIndex == INVALID_WORD)) return false;

		//断开用户
		if (pIServerUserItem->GetBindIndex() != INVALID_WORD)
		{
			//发送通知
			SendRoomMessage(pIServerUserItem, "请注意，您的帐号在另一地方进入了此游戏房间，您被迫离开！", SMT_CHAT | SMT_EJECT | SMT_GLOBAL | SMT_CLOSE_ROOM);

			//绑定参数
			uint16 wSourceIndex = pIServerUserItem->GetBindIndex();
			tagBindParameter * pSourceParameter = GetBindParameter(wSourceIndex);

			//解除绑定
			assert((pSourceParameter != nullptr) && (pSourceParameter->pIServerUserItem == pIServerUserItem));
			if ((pSourceParameter != nullptr) && (pSourceParameter->pIServerUserItem == pIServerUserItem)) pSourceParameter->pIServerUserItem = nullptr;

			//断开用户
			m_pITCPNetworkEngine->ShutDownSocket(pSourceParameter->dwSocketID);
		}

		//机器判断
		char* tszMachineID = pIServerUserItem->GetMachineID();
		bool bSameMachineID = strcmp(szMachineID, tszMachineID) == 0;

		//激活用户
		tagBindParameter * pTargetParameter = GetBindParameter(wTargetIndex);
		pTargetParameter->pIServerUserItem = pIServerUserItem;
		pIServerUserItem->SetUserParameter(pTargetParameter->dwClientAddr, wTargetIndex, szMachineID, false);

		//状态切换
		bool bIsOffLine = false;
		if (pIServerUserItem->GetUserStatus() == US_OFFLINE)
		{
			//变量定义
			uint16 wTableID = pIServerUserItem->GetTableID();
			uint16 wChairID = pIServerUserItem->GetChairID();

			//设置状态
			bIsOffLine = true;
			pIServerUserItem->SetUserStatus(US_PLAYING, wTableID, wChairID);
		}

		//登录事件
		OnEventUserLogon(pIServerUserItem, true);

		//安全提示
		if (!bIsOffLine && !bSameMachineID)
		{
			SendRoomMessage(pIServerUserItem, "请注意，您的帐号在另一地方进入了此游戏房间，对方被迫离开！", SMT_EJECT | SMT_CHAT | SMT_GLOBAL);
		}
		return true;
	}

	void CAttemperEngineSink::OnEventUserLogon(IRoomUserItem * pIServerUserItem, bool bOnLine)
	{
		//获取参数
		uint16 wBindIndex = pIServerUserItem->GetBindIndex();
		tagBindParameter * pBindParameter = GetBindParameter(wBindIndex);

		//变量定义
		CMD_GR_LogonSuccess LogonSuccess;
		memset(&LogonSuccess, 0, sizeof(LogonSuccess));

		//登录成功
		LogonSuccess.dwUserRight = 0;//pIServerUserItem->GetUserRight();
		LogonSuccess.dwMasterRight = 0;//pIServerUserItem->GetMasterRight();
		SendData(pBindParameter->dwSocketID, MDM_GR_LOGON, SUB_GR_LOGON_SUCCESS, &LogonSuccess, sizeof(LogonSuccess));

		//变量定义
		CMD_GR_ConfigServer ConfigServer;
		memset(&ConfigServer, 0, sizeof(ConfigServer));

		//房间配置
		ConfigServer.wTableCount = m_pGameServiceOption->wTableCount;
		ConfigServer.wChairCount = m_pGameServiceOption->wChairCount;
		ConfigServer.wServerType = 0;// m_pGameServiceOption->wServerType;
		ConfigServer.dwServerRule = 0;// m_pGameServiceOption->dwServerRule;
		SendData(pBindParameter->dwSocketID, MDM_GR_CONFIG, SUB_GR_CONFIG_SERVER, &ConfigServer, sizeof(ConfigServer));

		//配置完成
		SendData(pBindParameter->dwSocketID, MDM_GR_CONFIG, SUB_GR_CONFIG_FINISH, nullptr, 0);

		//发送信息
		SendUserInfoPacket(pIServerUserItem, pBindParameter->dwSocketID);

		//发送其他	
		CRoomUserManager::CRoomUserItemMap ruim = m_ServerUserManager.TraverseRoomUserList();
		for (CRoomUserManager::RUIM_IT it = ruim.begin(); it != ruim.end(); ++it)
		{
			if (it->second == pIServerUserItem) continue;
			SendUserInfoPacket(it->second, pBindParameter->dwSocketID);
		}

		//发送信息
		SendUserInfoPacketBatch(pIServerUserItem, INVALID_DWORD);

		//登录完成
		SendData(pBindParameter->dwSocketID, MDM_GR_LOGON, SUB_GR_LOGON_FINISH, nullptr, 0);

		//桌子状态
		CMD_GR_TableInfo TableInfo;
		TableInfo.wTableCount = (uint16)m_TableFrameArray.size();
		assert(TableInfo.wTableCount < ARR_LEN(TableInfo.TableStatusArray));
		for (uint16 i = 0; i < TableInfo.wTableCount; i++)
		{
			CTableFrame * pTableFrame = m_TableFrameArray[i];
			TableInfo.TableStatusArray[i].cbTableLock = 0;
			TableInfo.TableStatusArray[i].cbPlayStatus = 0;
			TableInfo.TableStatusArray[i].lCellScore = 1;
		}

		//桌子状态
		uint16 wHeadSize = sizeof(TableInfo) - sizeof(TableInfo.TableStatusArray);
		uint16 wSendSize = wHeadSize + TableInfo.wTableCount * sizeof(TableInfo.TableStatusArray[0]);
		SendData(pBindParameter->dwSocketID, MDM_GR_STATUS, SUB_GR_TABLE_INFO, &TableInfo, wSendSize);

		//网络设置
		m_pITCPNetworkEngine->AllowBatchSend(pBindParameter->dwSocketID, true);
		return;
	}

	void CAttemperEngineSink::OnEventUserLogout(IRoomUserItem * pIServerUserItem, uint32 dwLeaveReason)
	{
		//TODO...DB

		//汇总用户
		{
			//变量定义
			CMD_CS_C_UserLeave UserLeave;
			memset(&UserLeave, 0, sizeof(UserLeave));

			//设置变量
			UserLeave.dwUserID = pIServerUserItem->GetUserID();

			//发送消息
			m_pITCPSocketService->SendData(MDM_CS_USER_COLLECT, SUB_CS_C_USER_LEAVE, &UserLeave, sizeof(UserLeave));
		}

		//删除用户
		m_ServerUserManager.DeleteUserItem(pIServerUserItem);
	}

	IRoomUserItem * CAttemperEngineSink::GetBindUserItem(uint16 wBindIndex)
	{
		//获取参数
		tagBindParameter * pBindParameter = GetBindParameter(wBindIndex);

		//获取用户
		if (pBindParameter != nullptr)
		{
			return pBindParameter->pIServerUserItem;
		}

		//错误断言
		assert(nullptr);
		return nullptr;
	}

	tagBindParameter * CAttemperEngineSink::GetBindParameter(uint16 wBindIndex)
	{
		//无效连接
		if (wBindIndex == INVALID_WORD) return nullptr;

		//常规连接
		if (wBindIndex < m_pGameServiceOption->wMaxPlayer)
		{
			return m_pBindParameter + wBindIndex;
		}

		//错误断言
		assert(nullptr);
		return nullptr;
	}

	bool CAttemperEngineSink::SendLogonFailure(const char * pszString, uint32 lErrorCode, uint32 dwSocketID)
	{
		CMD_GR_LogonFailure LogonFailure;
		memset(&LogonFailure, 0, sizeof(LogonFailure));

		LogonFailure.lResultCode = lErrorCode;
		
		std::wstring wstrLogonError = Util::StringUtility::StringToWString(pszString);
		swprintf(LogonFailure.szDescribeString, sizeof(LogonFailure.szDescribeString), L"%s", wstrLogonError.c_str());

		return m_pITCPNetworkEngine->SendData(dwSocketID, MDM_GR_LOGON, SUB_GR_LOGON_FAILURE, &LogonFailure, sizeof(LogonFailure));
	}

	bool CAttemperEngineSink::SendUserFailure(IRoomUserItem * pIServerUserItem, const char * pszDescribe, uint32 lErrorCode)
	{
		//变量定义
		CMD_GR_UserRequestFailure UserRequestFailure;
		memset(&UserRequestFailure, 0, sizeof(UserRequestFailure));

		//设置变量
		UserRequestFailure.lErrorCode = lErrorCode;
		
		std::wstring wstrDescribe = Util::StringUtility::StringToWString(pszDescribe);
		swprintf(UserRequestFailure.szDescribeString, sizeof(UserRequestFailure.szDescribeString), L"%s", wstrDescribe.c_str());

		//发送数据
		SendData(pIServerUserItem, MDM_GR_USER, SUB_GR_USER_REQUEST_FAILURE, &UserRequestFailure, sizeof(UserRequestFailure));
		return true;
	}

	bool CAttemperEngineSink::QuerryGameRoomRecordTime(IRoomUserItem * pIServerUserItem)
	{
		//红黑
		if (m_pGameServiceOption->wKindID == 104)
		{
			for (size_t i = 0; i < m_TableFrameArray.size(); ++i)
			{
				CTableFrame * pTableFrame = m_TableFrameArray[i];

				CMD_GF_RBRoomStatus RoomStatus;
				memset(&RoomStatus, 0, sizeof(RoomStatus));
				pTableFrame->OnGetGameRecord(&RoomStatus);
				SendData(pIServerUserItem, MDM_GF_FRAME, SUB_GF_BJL_GAME_STATUS, &RoomStatus, sizeof(RoomStatus));
			}
			SendData(pIServerUserItem, MDM_GF_FRAME, SUB_GF_BAIREN_STATUS_END, nullptr, 0);
		}
		return true;
	}

	bool CAttemperEngineSink::PerformUnlockScore(uint32 dwUserID, uint32 dwInoutIndex, uint32 dwLeaveReason)
	{
		return false;
	}
}