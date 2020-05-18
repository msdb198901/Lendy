#include "AttemperEngineSink.h"
#include "ServiceUnits.h"
#include "CMD_LogonServer.h"
#include "CMD_Correspond.h"
#include "Implementation/LogonDatabase.h"
#include "Log.h"
#include "INIReader.h"
#include "StringUtility.h"

#define MAX_LINK_COUNT 512
#define OPEN_SWITCH		1
#define CLIENT_SWITCH	0

namespace Logon
{
	using namespace LogComm;

#define IDI_LOAD_GAME_LIST			1									//加载列表
#define IDI_CONNECT_CORRESPOND		2									//重连标识

#define LOGON_FAILURE(linkid, errorcode) \
	if (OnLogonFailure(linkid, errorcode)) { \
		return true;	\
	}

	CAttemperEngineSink::CAttemperEngineSink()
	{
		
	}

	CAttemperEngineSink::~CAttemperEngineSink()
	{
	}

	void CAttemperEngineSink::Release()
	{
	}

	void * CAttemperEngineSink::QueryInterface(GGUID uuid)
	{
		QUERY_INTERFACE(IAttemperEngineSink, uuid);
		QUERY_INTERFACE_IUNKNOWNEX(IAttemperEngineSink, uuid);
		return nullptr;
	}

	bool CAttemperEngineSink::OnAttemperEngineStart(IUnknownEx * pIUnknownEx)
	{
		m_pBindParameter = new tagBindParameter[MAX_LINK_COUNT];
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
				LOG_INFO("server.logon", "Correspond server connection failed [ %d ], will reconnect in %d seconds", iErrorCode, iConnectTime);
				m_pITimerEngine->SetTimer(IDI_CONNECT_CORRESPOND, iConnectTime * 1000, 1);
				return false;
			}

			//提示消息
			LOG_INFO("server.logon", "Registering Loginserver...");

			//变量定义
			CMD_CS_C_RegisterLogon RegisterLogon;
			memset(&RegisterLogon, 0, sizeof(RegisterLogon));

			//设置变量
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
			sprintf_s(RegisterLogon.szServerName, "%s", sConfigMgr->Get("LocalNet", "Name", "").c_str());
			sprintf_s(RegisterLogon.szServerAddr, "%s", sConfigMgr->Get("LocalNet", "WinBindIP", "").c_str());
#else
			snprintf(RegisterLogon.szServerName, sizeof(RegisterLogon.szServerName), "%s", sConfigMgr->Get("LocalNet", "Name", "").c_str());
			snprintf(RegisterLogon.szServerAddr, sizeof(RegisterLogon.szServerAddr), "%s", sConfigMgr->Get("LocalNet", "LinuxBindIP", "").c_str());
#endif

			//发送数据
			m_pITCPSocketService->SendData(MDM_CS_REGISTER, SUB_CS_C_REGISTER_LOGON, &RegisterLogon, sizeof(RegisterLogon));

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
			//if (m_bNeekCorrespond == true)
			{
				//提示消息
				int iConnectTime = sConfigMgr->GetInt32("LocalNet", "ConnectTime", 5);
				LOG_INFO("server.logon", "The connection to the correspond server is closed, and will reconnect in %d seconds", iConnectTime);

				//设置时间
				assert(m_pITimerEngine != nullptr);
				m_pITimerEngine->SetTimer(IDI_CONNECT_CORRESPOND, iConnectTime * 1000, 1);
				return true;
			}
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
		memset((m_pBindParameter + dwSocketID), 0, sizeof(tagBindParameter));
		return true;
	}

	bool CAttemperEngineSink::OnEventTCPNetworkRead(Net::TCP_Command Command, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		switch (Command.wMainCmdID)
		{
			case MDM_MB_LOGON:			//登录命令
			{
				return OnTCPNetworkMainMBLogon(Command.wSubCmdID, pData, wDataSize, dwSocketID);
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
				//查询类型
				tagGameKind GameKind = {};
				GameKind.wKindID = 104;
				GameKind.wGameID = 104;

				GameKind.dwOnLineCount = 0;
				GameKind.dwAndroidCount = 0;
				GameKind.dwFullCount = 100;
				m_RoomListManager.InsertGameKind(&GameKind);

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

	//时间事件
	bool CAttemperEngineSink::OnEventTimer(uint32 dwTimerID)
	{
		switch (dwTimerID)
		{
			case IDI_CONNECT_CORRESPOND:
			{
				std::string strCorrespondAddress = sConfigMgr->Get("CorrespondNet", "BindIP", "127.0.0.1");
				uint16 wCorrespondPort = sConfigMgr->GetInt32("CorrespondNet", "Port", 8600);
				m_pITCPSocketService->Connect(strCorrespondAddress, wCorrespondPort);
				LOG_INFO("server.logon", "Connecting to the correspond server [ %s:%d ]", strCorrespondAddress, wCorrespondPort);
				return true;
			}
		}
		return false;
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
				//m_bNeekCorrespond = false;
				m_pITCPSocketService->CloseSocket();

				//显示消息
				LOG_INFO("server.logon", "%s", pRegisterFailure->szDescribeString);

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
				m_RoomListManager.DisuseRoomItem();
				return true;
			}
			case SUB_CS_S_ROOM_ONLINE:	//房间人数
			{
				//效验参数
				assert(wDataSize == sizeof(CMD_CS_S_RoomOnLine));
				if (wDataSize != sizeof(CMD_CS_S_RoomOnLine)) return false;

				//变量定义
				CMD_CS_S_RoomOnLine * pServerOnLine = (CMD_CS_S_RoomOnLine *)pData;

				tagGameRoom *pRoomItem = m_RoomListManager.SearchGameRoom(pServerOnLine->wServerID);
				if (pRoomItem == nullptr) return true;

				uint32 dwOldOnlineCount = 0, dwOldAndroidCount = 0;
				dwOldOnlineCount = pRoomItem->dwOnLineCount;
				dwOldAndroidCount = pRoomItem->dwAndroidCount;

				//目录人数
				tagGameKind * pGameKindItem = m_RoomListManager.SearchGameKind(pRoomItem->wKindID);
				if (pGameKindItem != NULL)
				{
					//在线人数
					pGameKindItem->dwOnLineCount -= dwOldOnlineCount;
					pGameKindItem->dwOnLineCount += pRoomItem->dwOnLineCount;

					//机器人数
					pGameKindItem->dwAndroidCount -= dwOldAndroidCount;
					pGameKindItem->dwAndroidCount += pRoomItem->dwAndroidCount;
				}
				return true;
			}
			case SUB_CS_S_ROOM_REMOVE:	//房间删除
			{
				//效验参数
				assert(wDataSize == sizeof(CMD_CS_S_RoomRemove));
				if (wDataSize != sizeof(CMD_CS_S_RoomRemove)) return false;

				//变量定义
				CMD_CS_S_RoomRemove * pRoomRemove = (CMD_CS_S_RoomRemove *)pData;

				m_RoomListManager.DeleteGameRoom(pRoomRemove->wServerID);
				return true;
			}
			case SUB_CS_S_ROOM_INSERT:	//房间插入
			{
				//效验参数
				assert(wDataSize % sizeof(tagGameRoom) == 0);
				if (wDataSize % sizeof(tagGameRoom) != 0) return false;

				//变量定义
				uint16 wItemCount = wDataSize / sizeof(tagGameRoom);
				tagGameRoom * pGameRoom = (tagGameRoom *)pData;

				//更新数据
				for (uint16 i = 0; i < wItemCount; ++i)
				{
					m_RoomListManager.InsertGameRoom(pGameRoom++);
				}
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

	bool CAttemperEngineSink::OnTCPNetworkMainMBLogon(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		switch (wSubCmdID)
		{
			case SUB_MB_LOGON_VISITOR:      //游客登录
			{
				return OnTCPNetworkSubMBLogonVisitor(pData, wDataSize, dwSocketID);
			}
		}
		return false;
	}
	bool CAttemperEngineSink::OnTCPNetworkSubMBLogonVisitor(void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		//效验参数
		assert(wDataSize >= sizeof(CMD_MB_LogonVisitor));
		if (wDataSize < sizeof(CMD_MB_LogonVisitor))return false;

		//变量定义
		tagBindParameter * pBindParameter = (m_pBindParameter + dwSocketID);

		//处理消息
		CMD_MB_LogonVisitor * pLogonVisitor = (CMD_MB_LogonVisitor *)pData;

		//设置连接
		pBindParameter->cbClientKind = LinkType::LT_MOBILE;

		LogonErrorCode eLogonErrorCode = LEC_NONE;
		uint8 * pClientAddr = (uint8 *)&pBindParameter->dwClientAddr;
		std::string strClientIP = StringFormat("%d.%d.%d.%d", pClientAddr[3], pClientAddr[2], pClientAddr[1], pClientAddr[0]);

		////////////////////////////////////
		PreparedStatement *stmt = LogonDatabasePool.GetPreparedStatement(LOGON_SEL_LIMIT_ADDRESS);
		stmt->SetString(0, strClientIP);
		stmt->SetString(1, pLogonVisitor->szMachineID);
		PreparedQueryResult result = LogonDatabasePool.Query(stmt);

		if (result)
		{
			Field* field = result->Fetch();
			while (field[3].GetInt8() == OPEN_SWITCH)
			{
				if (field[2].GetUInt32() < time(0))
				{
					//更新禁止信息
					stmt = LogonDatabasePool.GetPreparedStatement(LOGON_UPD_LIMIT_ADDRESS);
					stmt->SetInt8(0, 0);
					stmt->SetInt8(1, 0);
					stmt->SetInt8(2, 0);
					stmt->SetString(3, strClientIP);
					stmt->SetString(4, pLogonVisitor->szMachineID);
					LogonDatabasePool.DirectExecute(stmt);
					break;
				}

				if (field[0].GetInt8() == OPEN_SWITCH)
				{
					eLogonErrorCode = LEC_LIMIT_IP;
					break;
				}

				if (field[1].GetInt8() == OPEN_SWITCH)
				{
					eLogonErrorCode = LEC_LIMIT_MAC;
					break;
				}
				
				LOG_ERROR("server.logon", "禁止登录逻辑出错 IP: %s  MAC: %s", strClientIP.c_str(), pLogonVisitor->szMachineID);
				break;
			}
		}

		//是否禁止登陆
		LOGON_FAILURE(dwSocketID, eLogonErrorCode)

		//查询用户信息
		stmt = LogonDatabasePool.GetPreparedStatement(LOGON_SEL_VISITOR_ACCOUNT);
		stmt->SetString(0, pLogonVisitor->szMachineID);
		result = LogonDatabasePool.Query(stmt);
		if (!result)
		{
			int game_id = 0;
			stmt = LogonDatabasePool.GetPreparedStatement(LOGON_SEL_GAME_ID);
			PreparedQueryResult result_id = LogonDatabasePool.Query(stmt);
			if (result_id)
			{
				Field* field = result_id->Fetch();
				game_id = field[0].GetInt32();

				//更新标识
				stmt = LogonDatabasePool.GetPreparedStatement(LOGON_UPD_GAME_ID);
				LogonDatabasePool.DirectExecute(stmt);
			}
			else 
			{
				LOG_ERROR("server.logon", "分配游客ID出错 IP: %s  MAC: %s", strClientIP.c_str(), pLogonVisitor->szMachineID);
			}


			//插入游客用户
			std::string strVisitor = StringFormat("Visitor%d", game_id);
			stmt = LogonDatabasePool.GetPreparedStatement(LOGON_INS_VISITOR_ACCOUNT);
			
			std::string strUTF8Visitor;
			Util::StringUtility::ConsoleToUtf8(strVisitor, strUTF8Visitor);
			stmt->SetString(0, strUTF8Visitor);
			stmt->SetString(1, strUTF8Visitor);
			stmt->SetString(2, "");
			stmt->SetString(3, "1");
			stmt->SetInt8(4, 100/*pBindParameter->cbClientKind*/);
			stmt->SetString(5, strClientIP);
			stmt->SetString(6, pLogonVisitor->szMachineID);
			LogonDatabasePool.DirectExecute(stmt);

			//重新查询游客
			stmt = LogonDatabasePool.GetPreparedStatement(LOGON_SEL_VISITOR_ACCOUNT);
			stmt->SetString(0, pLogonVisitor->szMachineID);
			result = LogonDatabasePool.Query(stmt);
			if (!result) 
			{
				LOG_ERROR("server.logon", "Insert ID IP: %s  MAC: %s", strClientIP.c_str(), pLogonVisitor->szMachineID);
				return false;
			}
		}

		//获取游戏信息
		Field* field = result->Fetch();
		int id = field[0].GetInt32();
		std::string account = field[1].GetString();
		std::string username = field[2].GetString();
		std::string sha_pass_hash = field[3].GetString();
		std::string face_url = field[4].GetString();
		int limit = field[5].GetInt8();

		//账号冻结状态
		if ((limit & LEC_LIMIT_FREEZE) > 0)
		{
			eLogonErrorCode = LEC_LIMIT_FREEZE;
		}
		LOGON_FAILURE(dwSocketID, eLogonErrorCode)

		//更新登陆信息
		stmt = LogonDatabasePool.GetPreparedStatement(LOGON_UPD_VISITOR_ACCOUNT);
		stmt->SetString(0, strClientIP);
		stmt->SetString(1, pLogonVisitor->szMachineID);
		LogonDatabasePool.DirectExecute(stmt);

		//////////////////////////////////////////////////////////////////////////////////////////
		CMD_MB_LogonSuccess LogonSuccess;
		memset(&LogonSuccess, 0, sizeof(LogonSuccess));
		
		LogonSuccess.dwUserID = id;
		LogonSuccess.dwGameID = id;
		LogonSuccess.lUserScore = field[6].GetUInt64();
		
		std::string strAnsiAccount;
		Util::StringUtility::Utf8ToConsole(account, strAnsiAccount);
		std::wstring wstrAccount = Util::StringUtility::StringToWString(strAnsiAccount);
		swprintf((wchar_t*)LogonSuccess.szAccounts, sizeof(LogonSuccess.szAccounts), L"%ls", wstrAccount.c_str());

		std::string strAnsiUsername;
		Util::StringUtility::Utf8ToConsole(account, strAnsiUsername);
		std::wstring wstrUsername = Util::StringUtility::StringToWString(strAnsiUsername);
		swprintf((wchar_t*)LogonSuccess.szNickName, sizeof(LogonSuccess.szNickName), L"%ls", wstrAccount.c_str());

		std::string strAnsiSHAPass;
		Util::StringUtility::Utf8ToConsole(sha_pass_hash, strAnsiSHAPass);
		std::wstring wstrSHAPass = Util::StringUtility::StringToWString(sha_pass_hash);
		swprintf((wchar_t*)LogonSuccess.szDynamicPass, sizeof(LogonSuccess.szDynamicPass), L"%ls", wstrSHAPass.c_str());

		m_pITCPNetworkEngine->SendData(dwSocketID, MDM_MB_LOGON, SUB_MB_LOGON_SUCCESS, &LogonSuccess, sizeof(LogonSuccess));

		//SendKindListInfo(dwSocketID);
		SendRoomListInfo(dwSocketID, INVALID_WORD);
		m_pITCPNetworkEngine->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_LIST_FINISH);
		return true;
	}
	
	void CAttemperEngineSink::SendKindListInfo(uint32 dwSocketID)
	{
		//网络数据
		uint16 wSendSize = 0;
		uint8 cbDataBuffer[SOCKET_TCP_PACKET];

		//遍历数据
		KindItemMap kim = m_RoomListManager.TraverseKindList();
		for (KIM_IT it = kim.begin(); it != kim.end(); ++it)
		{
			//发送数据
			if ((wSendSize + sizeof(CMD_MB_GameKindItem)) > sizeof(cbDataBuffer))
			{
				m_pITCPNetworkEngine->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_KIND_LIST, cbDataBuffer, wSendSize);
				wSendSize = 0;
			}

			CMD_MB_GameKindItem GameKindItem = {};
			GameKindItem.wSortID = it->second->wSortID;
			GameKindItem.wKindID = it->second->wKindID;
			GameKindItem.wGameID = it->second->wGameID;

			GameKindItem.dwOnLineCount	= it->second->dwOnLineCount;
			GameKindItem.dwAndroidCount = it->second->dwAndroidCount;
			GameKindItem.dwFullCount	= it->second->dwFullCount;

			//////////////////////////////////////
			memcpy(cbDataBuffer + wSendSize, &GameKindItem, sizeof(GameKindItem));
			wSendSize += sizeof(GameKindItem);
		}

		//发送剩余
		if (wSendSize > 0) m_pITCPNetworkEngine->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_KIND_LIST, cbDataBuffer, wSendSize);
		return;
	}

	void CAttemperEngineSink::SendRoomListInfo(uint32 dwSocketID, uint16 wKindID)
	{
		//网络数据
		uint16 wSendSize = 0;
		uint8 cbDataBuffer[SOCKET_TCP_PACKET];

		//遍历数据
		RoomItemMap rim = m_RoomListManager.TraverseRoomList();
		for (RIM_IT it = rim.begin(); it != rim.end(); ++it)
		{
			//发送数据
			if ((wSendSize + sizeof(CMD_MB_GameRoomItem)) > sizeof(cbDataBuffer))
			{
				m_pITCPNetworkEngine->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_ROOM_LIST, cbDataBuffer, wSendSize);
				wSendSize = 0;
			}

			CMD_MB_GameRoomItem GameRoomItem = {};
			GameRoomItem.wKindID		= it->second->wKindID;
			GameRoomItem.wSortID		= it->second->wSortID;
			GameRoomItem.wServerID		= it->second->wServerID;
			GameRoomItem.wServerKind	= it->second->wServerKind;
			GameRoomItem.wServerLevel	= it->second->wServerLevel;
			GameRoomItem.wServerPort	= it->second->wServerPort;
			GameRoomItem.wTableCount	= (uint8)it->second->wTableCount;
			GameRoomItem.lCellScore		= it->second->lCellScore;
			GameRoomItem.lEnterScore	= it->second->lEnterScore;
								
			GameRoomItem.dwServerRule	= it->second->dwServerRule;
			
			GameRoomItem.dwOnLineCount	= it->second->dwOnLineCount;
			GameRoomItem.dwAndroidCount = it->second->dwAndroidCount;
			GameRoomItem.dwFullCount	= it->second->dwFullCount;
	
			std::wstring wstrServerAddr = Util::StringUtility::StringToWString(it->second->szServerName);
			swprintf((wchar_t*)GameRoomItem.szServerAddr, sizeof(GameRoomItem.szServerAddr), L"%ls", wstrServerAddr.c_str());

			std::wstring wstrServerName = Util::StringUtility::StringToWString(it->second->szServerAddr);
			swprintf((wchar_t*)GameRoomItem.szServerName, sizeof(GameRoomItem.szServerName), L"%ls", wstrServerName.c_str());

			if (wKindID == INVALID_WORD || wKindID == it->second->wKindID)
			{
				memcpy(cbDataBuffer + wSendSize, &GameRoomItem, sizeof(GameRoomItem));
				wSendSize += sizeof(GameRoomItem);
			}
		}

		if (wSendSize > 0) m_pITCPNetworkEngine->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_ROOM_LIST, cbDataBuffer, wSendSize);
	}

	bool CAttemperEngineSink::OnLogonFailure(uint32 dwSocketID, LogonErrorCode & lec)
	{
		if (lec == LEC_NONE)
		{
			return false;
		}

		CMD_MB_LogonFailure LogonFailure;
		memset(&LogonFailure, 0, sizeof(LogonFailure));

		LogonFailure.lResultCode = lec;
		std::wstring wstrLogonError = Util::StringUtility::StringToWString(LogonError[lec]);
		swprintf((wchar_t*)LogonFailure.szDescribe, sizeof(LogonFailure.szDescribe), L"%ls", wstrLogonError.c_str());

		return m_pITCPNetworkEngine->SendData(dwSocketID, MDM_MB_LOGON, SUB_MB_LOGON_FAILURE, &LogonFailure, sizeof(LogonFailure));
	}
}