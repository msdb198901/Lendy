#include "AttemperEngineSink.h"
#include "ServiceUnits.h"
#include "CMD_LogonServer.h"
#include "CMD_Correspond.h"
#include "Implementation/LogonDatabase.h"
#include "Log.h"
#include "StringUtility.h"

#define MAX_LINK_COUNT 512
#define OPEN_SWITCH		1
#define CLIENT_SWITCH	0

namespace Game
{
	using namespace LogComm;

#define LOGON_FAILURE(linkid, errorcode) \
	if (OnLogonFailure(linkid, errorcode)) { \
		return true;	\
	}

	CAttemperEngineSink::CAttemperEngineSink()
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
		QUERY_INTERFACE(IAttemperEngineSink, uuid);
		QUERY_INTERFACE_IUNKNOWNEX(IAttemperEngineSink, uuid);
		return nullptr;
	}

	bool CAttemperEngineSink::OnAttemperEngineStart(IUnknownEx * pIUnknownEx)
	{
		m_pBindParameter = new tagBindParameter[MAX_LINK_COUNT];
		
		InitTableFrameArray();
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
				LOG_INFO("server.logon", "协调服务器连接失败 [ %ld ]，%ld 秒后将重新连接", iErrorCode, 5);
				return false;
			}

			//提示消息
			LOG_INFO("server.logon", "正在注册游戏登录服务器...");

			//变量定义
			CMD_CS_C_RegisterPlaza RegisterPlaza;
			memset(&RegisterPlaza, 0, sizeof(RegisterPlaza));

			//设置变量
			std::string t = "登录服务器";
			std::wstring wstrLogon = Util::StringUtility::StringToWString(t);
			swprintf_s(RegisterPlaza.szServerName, L"%s", wstrLogon.c_str());

			std::string t1 = "192.168.1.217";
			std::wstring wstrIP = Util::StringUtility::StringToWString(t1);
			swprintf_s(RegisterPlaza.szServerAddr, L"%s", wstrIP.c_str());

			//发送数据
			m_pITCPSocketService->SendData(MDM_CS_REGISTER, SUB_CS_C_REGISTER_PLAZA, &RegisterPlaza, sizeof(RegisterPlaza));

			return true;
		}

		return true;
	}

	bool CAttemperEngineSink::OnEventTCPSocketShut(uint16 wServiceID, uint8 cbShutReason)
	{
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
				case MDM_CS_SERVICE_INFO:	//服务信息
				{
					return OnTCPSocketMainServiceInfo(Command.wSubCmdID, pData, wDataSize);
				}
			}
		}
		return false;
	}

	bool CAttemperEngineSink::OnEventTCPNetworkBind(uint64 dwClientAddr, uint64 dwSocketID)
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

	bool CAttemperEngineSink::OnEventTCPNetworkShut(uint64 dwClientAddr, uint64 dwSocketID)
	{
		memset((m_pBindParameter + dwSocketID), 0, sizeof(tagBindParameter));
		return true;
	}

	bool CAttemperEngineSink::OnEventTCPNetworkRead(Net::TCP_Command Command, void * pData, uint16 wDataSize, uint64 dwSocketID)
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
				//事件通知
				ControlResult ControlResult;
				ControlResult.cbSuccess = 1;
				SrvUnitsMgr->PostControlRequest(UDC_LOAD_DB_LIST_RESULT, &ControlResult, sizeof(ControlResult));
				return true;
			}
			case SUC_CONNECT_CORRESPOND:
			{
				//发起连接
				std::string str = "192.168.1.217";
				m_pITCPSocketService->Connect(str, 8610);
				return true;
			}
		}
		return false;
	}

	bool CAttemperEngineSink::InitTableFrameArray()
	{
		tagTableFrameParameter TableFrameParameter;
		memset(&TableFrameParameter, 0, sizeof(TableFrameParameter));

		//服务组件
		TableFrameParameter.pIGameServiceManager = m_pIGameServiceManager;
		TableFrameParameter.pITCPSocketService = m_pITCPSocketService;

		//配置参数
		TableFrameParameter.pGameServiceOption = m_pGameServiceOption;

		//桌子容器
		m_TableFrameArray.resize(60);// m_pGameServiceOption->wTableCount);

		//创建桌子
		for (uint16 i = 0; i < 60; ++i)
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
				//m_bNeekCorrespond = false;
				m_pITCPSocketService->CloseSocket();

				//显示消息
				std::string strResult = Util::StringUtility::WStringToString(pRegisterFailure->szDescribeString);

				LOG_INFO("server.logon", "%d", strResult.c_str());

				//事件通知
				//CP_ControlResult ControlResult;
				//ControlResult.cbSuccess = ER_FAILURE;
				//SendUIControlPacket(UI_CORRESPOND_RESULT, &ControlResult, sizeof(ControlResult));
				return true;
			}
		}

		return true;
	}

	bool CAttemperEngineSink::OnTCPSocketMainServiceInfo(uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		switch (wSubCmdID)
		{
			case SUB_CS_S_SERVER_INFO:		//房间信息
			{
				return true;
			}
			case SUB_CS_S_SERVER_ONLINE:	//房间人数
			{
				//效验参数
				assert(wDataSize == sizeof(CMD_CS_S_ServerOnLine));
				if (wDataSize != sizeof(CMD_CS_S_ServerOnLine)) return false;

				//变量定义
				CMD_CS_S_ServerOnLine * pServerOnLine = (CMD_CS_S_ServerOnLine *)pData;
				return true;
			}
			case SUB_CS_S_SERVER_INSERT:	//房间插入
			{
				return true;
			}
			case SUB_CS_S_SERVER_FINISH:	//房间完成
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

	bool CAttemperEngineSink::OnTCPNetworkMainMBLogon(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint64 dwSocketID)
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
	bool CAttemperEngineSink::OnTCPNetworkSubMBLogonVisitor(void * pData, uint16 wDataSize, uint64 dwSocketID)
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
		BYTE * pClientAddr = (BYTE *)&pBindParameter->dwClientAddr;
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
			std::string strVisitor = StringFormat("游客%d", game_id);
			stmt = LogonDatabasePool.GetPreparedStatement(LOGON_INS_VISITOR_ACCOUNT);
			
			std::string strUTF8Visitor;
			Util::StringUtility::ConsoleToUtf8(strVisitor, strUTF8Visitor);
			stmt->SetString(0, strUTF8Visitor);
			stmt->SetString(1, strUTF8Visitor);
			stmt->SetString(2, "");
			stmt->SetString(3, "1");
			stmt->SetInt8(4, pBindParameter->cbClientKind);
			stmt->SetString(5, strClientIP);
			stmt->SetString(6, pLogonVisitor->szMachineID);
			LogonDatabasePool.DirectExecute(stmt);

			//重新查询游客
			stmt = LogonDatabasePool.GetPreparedStatement(LOGON_SEL_VISITOR_ACCOUNT);
			stmt->SetString(0, pLogonVisitor->szMachineID);
			result = LogonDatabasePool.Query(stmt);
			if (!result) 
			{
				LOG_ERROR("server.logon", "插入游客ID出错 IP: %s  MAC: %s", strClientIP.c_str(), pLogonVisitor->szMachineID);
				return false;
			}
		}

		//获取游戏信息
		Field* field = result->Fetch();
		int id = field[0].GetInt32();
		std::string account = field[1].GetString();
		std::string username = field[2].GetString();
		std::string strAnsiVisitor;
		Util::StringUtility::Utf8ToConsole(account, strAnsiVisitor);
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

		return true;
	}
	
	bool CAttemperEngineSink::OnLogonFailure(uint64 dwSocketID, LogonErrorCode & lec)
	{
		if (lec == LEC_NONE)
		{
			return false;
		}

		CMD_MB_LogonFailure LogonFailure;
		memset(&LogonFailure, 0, sizeof(LogonFailure));

		LogonFailure.lResultCode = lec;
		std::wstring wstrLogonError = Util::StringUtility::StringToWString(LogonError[lec]);
		swprintf_s(LogonFailure.szDescribe, L"%s", wstrLogonError.c_str());

		return m_pITCPNetworkEngine->SendData(dwSocketID, MDM_MB_LOGON, SUB_MB_LOGON_FAILURE, &LogonFailure, sizeof(LogonFailure));
	}
}