#include "AttemperEngineSink.h"
#include "ServiceUnits.h"
#include "CMD_Correspond.h"
#include "Log.h"
#include "Struct.h"
#include "StringUtility.h"

#define MAX_LINK_COUNT 512
#define OPEN_SWITCH		1
#define CLIENT_SWITCH	0

namespace Correspond
{
	//using namespace Comm;
	using namespace LogComm;

	CAttemperEngineSink::CAttemperEngineSink()
	{
		m_wCollectItem = INVALID_WORD;

		m_pBindParameter = nullptr;
		m_pITCPNetworkEngine = nullptr;
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
		m_wCollectItem = INVALID_WORD;
		m_WaitCollectItemArray.clear();

		PDELETE(m_pBindParameter);
		m_pITCPNetworkEngine = nullptr;
		return true;
	}

	bool CAttemperEngineSink::OnEventTCPSocketLink(uint16 wServiceID, int iErrorCode)
	{
		return false;
	}

	bool CAttemperEngineSink::OnEventTCPSocketShut(uint16 wServiceID, uint8 cbShutReason)
	{
		return false;
	}

	bool CAttemperEngineSink::OnEventTCPSocketRead(uint16 wServiceID, TCP_Command Command, void * pData, uint16 wDataSize)
	{
		return true;
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
		//获取信息
		uint16 wBindIndex = LOWORD(dwSocketID);
		tagBindParameter * pBindParameter = (m_pBindParameter + wBindIndex);

		//游戏服务
		if (pBindParameter->ServiceKind == ServiceKind_Game)
		{
			//用户汇总
			if (wBindIndex == m_wCollectItem)
			{
				//设置变量
				m_wCollectItem = INVALID_WORD;

				if (!m_WaitCollectItemArray.empty())
				{
					m_wCollectItem = m_WaitCollectItemArray.back();
					m_WaitCollectItemArray.pop_back();

					//发送消息
					uint32 dwSocketID = (m_pBindParameter + m_wCollectItem)->dwSocketID;
					m_pITCPNetworkEngine->SendData(dwSocketID, MDM_CS_USER_COLLECT, SUB_CS_S_COLLECT_REQUEST);
				}
			}
			else
			{
				//删除等待
				for (std::vector<uint16>::iterator it = m_WaitCollectItemArray.begin(); it != m_WaitCollectItemArray.end(); ++it)
				{
					if (*it == wBindIndex)
					{
						m_WaitCollectItemArray.erase(it);
						break;
					}
				}
			}

			//变量定义
			CMD_CS_S_RoomRemove RoomRemove;
			memset(&RoomRemove, 0, sizeof(RoomRemove));

			//删除通知
			RoomRemove.wServerID = pBindParameter->wServiceID;
			m_pITCPNetworkEngine->SendDataBatch(MDM_CS_ROOM_INFO, SUB_CS_S_ROOM_REMOVE, &RoomRemove, sizeof(RoomRemove));

			//注销房间
			m_GlobalInfoManager.DeleteRoomItem(pBindParameter->wServiceID);
		}

		//广场服务
		if (pBindParameter->ServiceKind == ServiceKind_Plaza)
		{
			m_GlobalInfoManager.DeleteLogonItem(pBindParameter->wServiceID);
		}

		//聊天服务
		if (pBindParameter->ServiceKind == ServiceKind_Chat)
		{
		}

		//清除信息
		memset(pBindParameter, 0, sizeof(tagBindParameter));
		return true;
	}

	bool CAttemperEngineSink::OnEventTCPNetworkRead(TCP_Command Command, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		switch (Command.wMainCmdID)
		{
			case MDM_CS_REGISTER:		//服务注册
			{
				return OnTCPNetworkMainRegister(Command.wSubCmdID, pData, wDataSize, dwSocketID);
			}
		}
		return false;
	}
	bool CAttemperEngineSink::OnEventControl(uint16 wControlID, void * pData, uint16 wDataSize)
	{
		return false;
	}

	bool CAttemperEngineSink::OnTCPNetworkMainRegister(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		switch (wSubCmdID)
		{
			case SUB_CS_C_REGISTER_LOGON:	//注册广场
			{
				//效验数据
				assert(wDataSize == sizeof(CMD_CS_C_RegisterLogon));
				if (wDataSize != sizeof(CMD_CS_C_RegisterLogon)) return false;

				//消息定义
				CMD_CS_C_RegisterLogon * pRegisterLogon = (CMD_CS_C_RegisterLogon *)pData;

				//有效判断
				if ((pRegisterLogon->szServerName[0] == 0) || (pRegisterLogon->szServerAddr[0] == 0))
				{
					//变量定义
					CMD_CS_S_RegisterFailure RegisterFailure;
					memset(&RegisterFailure, 0, sizeof(RegisterFailure));

					//设置变量
					RegisterFailure.lErrorCode = 0L;
					sprintf_s(RegisterFailure.szDescribeString, "服务器注册失败，“服务地址”与“服务器名”不合法！");

					//发送消息
					uint16 wStringSize = strlen(RegisterFailure.szDescribeString) + 1;
					uint16 wSendSize = sizeof(RegisterFailure) - sizeof(RegisterFailure.szDescribeString) + wStringSize;
					m_pITCPNetworkEngine->SendData(dwSocketID, MDM_CS_REGISTER, SUB_CS_S_REGISTER_FAILURE, &RegisterFailure, wSendSize);

					//中断网络
					m_pITCPNetworkEngine->ShutDownSocket(dwSocketID);
					return true;
				}

				//设置绑定
				uint16 wBindIndex = LOWORD(dwSocketID);
				(m_pBindParameter + wBindIndex)->wServiceID = wBindIndex;
				(m_pBindParameter + wBindIndex)->ServiceKind = ServiceKind_Plaza;

				//变量定义
				tagGameLogon GameLogon;
				memset(&GameLogon, 0, sizeof(GameLogon));

				//构造数据
				GameLogon.wPlazaID = wBindIndex;
				sprintf_s(GameLogon.szServerName, pRegisterLogon->szServerName, sizeof(GameLogon.szServerName));
				sprintf_s(GameLogon.szServerAddr, pRegisterLogon->szServerAddr, sizeof(GameLogon.szServerAddr));

				//注册房间
				m_GlobalInfoManager.ActiveLogonItem(GameLogon);

				//发送列表
				SendRoomListToLogon(dwSocketID);

				//群发设置
				m_pITCPNetworkEngine->AllowBatchSend(dwSocketID, true);

				return true;
			}
			case SUB_CS_C_REGISTER_ROOM:
			{
				//效验数据
				assert(wDataSize == sizeof(CMD_CS_C_RegisterRoom));
				if (wDataSize != sizeof(CMD_CS_C_RegisterRoom)) return false;

				//消息定义
				CMD_CS_C_RegisterRoom * pRegisterRoom = (CMD_CS_C_RegisterRoom *)pData;

				//查找房间
				if (m_GlobalInfoManager.SearchRoomItem(pRegisterRoom->wServerID) != nullptr)
				{
					//变量定义
					CMD_CS_S_RegisterFailure RegisterFailure;
					memset(&RegisterFailure, 0, sizeof(RegisterFailure));

					//设置变量
					RegisterFailure.lErrorCode = 0L;
					sprintf_s(RegisterFailure.szDescribeString, "已经存在相同标识的游戏房间服务，房间服务注册失败");

					//发送消息
					uint16 wStringSize = strlen(RegisterFailure.szDescribeString) + 1;
					uint16 wSendSize = sizeof(RegisterFailure) - sizeof(RegisterFailure.szDescribeString) + wStringSize;
					m_pITCPNetworkEngine->SendData(dwSocketID, MDM_CS_REGISTER, SUB_CS_S_REGISTER_FAILURE, &RegisterFailure, wSendSize);

					//中断网络
					m_pITCPNetworkEngine->ShutDownSocket(dwSocketID);

					return true;
				}

				//设置绑定
				WORD wBindIndex = LOWORD(dwSocketID);
				(m_pBindParameter + wBindIndex)->ServiceKind = ServiceKind_Game;
				(m_pBindParameter + wBindIndex)->wServiceID = pRegisterRoom->wServerID;

				tagGameRoom GameRoom;
				memset(&GameRoom, 0, sizeof(tagGameRoom));

				GameRoom.wKindID = pRegisterRoom->wKindID;
				GameRoom.wServerID = pRegisterRoom->wServerID;
				GameRoom.wServerPort = pRegisterRoom->wServerPort;
				GameRoom.lCellScore = pRegisterRoom->lCellScore;
				GameRoom.lEnterScore = pRegisterRoom->lEnterScore;
				GameRoom.dwOnLineCount = pRegisterRoom->dwOnLineCount;
				GameRoom.dwFullCount = pRegisterRoom->dwFullCount;
				GameRoom.wTableCount = pRegisterRoom->wTableCount;
				GameRoom.dwServerRule = pRegisterRoom->dwServerRule;
				sprintf_s(GameRoom.szServerAddr, pRegisterRoom->szServerAddr, sizeof(GameRoom.szServerAddr));
				sprintf_s(GameRoom.szServerName, pRegisterRoom->szServerName, sizeof(GameRoom.szServerName));

				//注册房间
				m_GlobalInfoManager.ActiveRoomItem(GameRoom);

				//群发房间
				m_pITCPNetworkEngine->SendDataBatch(MDM_CS_ROOM_INFO, SUB_CS_S_ROOM_INSERT, &GameRoom, sizeof(GameRoom));

				//汇总通知
				if (m_wCollectItem == INVALID_WORD)
				{
					m_wCollectItem = wBindIndex;
					m_pITCPNetworkEngine->SendData(dwSocketID, MDM_CS_USER_COLLECT, SUB_CS_S_COLLECT_REQUEST);
				}
				else m_WaitCollectItemArray.emplace_back(wBindIndex);
				return true;
			}
		}
		return false;
	}

	bool CAttemperEngineSink::OnTCPNetworkMainServiceInfo(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		return false;
	}

	bool CAttemperEngineSink::OnTCPNetworkMainUserCollect(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		return false;
	}

	bool CAttemperEngineSink::OnTCPNetworkMainRemoteService(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		return false;
	}

	bool CAttemperEngineSink::OnTCPNetworkMainManagerService(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		return false;
	}

	bool CAttemperEngineSink::OnTCPNetworkMainAndroidService(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID)
	{
		return false;
	}

	bool CAttemperEngineSink::SendRoomListToLogon(uint32 dwSocketID)
	{
		uint16 wPacketSize = 0;
		uint8 cbBuffer[SOCKET_TCP_PACKET];

		//发送信息
		m_pITCPNetworkEngine->SendData(dwSocketID, MDM_CS_ROOM_INFO, SUB_CS_S_ROOM_INFO);

		//获取对象
		tagGameRoom * pGameRoom = (tagGameRoom*)  (cbBuffer + wPacketSize);
		CGlobalInfoManager::ActiveGameRoomContainer agrc = m_GlobalInfoManager.TraverseGameRoom();
		for (CGlobalInfoManager::AGRC_IT it = agrc.begin(); it != agrc.end(); ++it)
		{
			//发送数据
			if ((wPacketSize + sizeof(tagGameRoom)) > sizeof(cbBuffer))
			{
				m_pITCPNetworkEngine->SendData(dwSocketID, MDM_CS_ROOM_INFO, SUB_CS_S_ROOM_INSERT, cbBuffer, wPacketSize);
				wPacketSize = 0;
			}

			wPacketSize += sizeof(tagGameRoom);
			memcpy(pGameRoom, it->second, sizeof(tagGameRoom));
		}

		//发送数据
		if (wPacketSize > 0) m_pITCPNetworkEngine->SendData(dwSocketID, MDM_CS_ROOM_INFO, SUB_CS_S_ROOM_INSERT, cbBuffer, wPacketSize);

		//发送完成
		m_pITCPNetworkEngine->SendData(dwSocketID, MDM_CS_ROOM_INFO, SUB_CS_S_ROOM_FINISH);
		return true;
	}
}