#include "RoomUserManager.h"

namespace Game
{
	CRoomUserItem::CRoomUserItem()
	{
	}
	CRoomUserItem::~CRoomUserItem()
	{
	}
	void * CRoomUserItem::QueryInterface(GGUID uuid)
	{
		return nullptr;
	}
	uint16 CRoomUserItem::GetBindIndex()
	{
		return m_wBindIndex;
	}
	uint64 CRoomUserItem::GetClientAddr()
	{
		return m_dwClientAddr;
	}
	char * CRoomUserItem::GetMachineID()
	{
		return m_szMachineID;
	}
	uint64 CRoomUserItem::GetDBQuestID()
	{
		return uint64();
	}
	uint64 CRoomUserItem::GetLogonTime()
	{
		return uint64();
	}
	uint64 CRoomUserItem::GetInoutIndex()
	{
		return uint64();
	}
	tagUserInfo * CRoomUserItem::GetUserInfo()
	{
		return &m_UserInfo;
	}
	uint8 CRoomUserItem::GetGender()
	{
		return m_UserInfo.cbGender;
	}
	uint32 CRoomUserItem::GetUserID()
	{
		return m_UserInfo.dwUserID;
	}
	uint32 CRoomUserItem::GetGameID()
	{
		return m_UserInfo.dwGameID;
	}
	char * CRoomUserItem::GetNickName()
	{
		return m_UserInfo.szNickName;
	}
	uint16 CRoomUserItem::GetTableID()
	{
		return m_UserInfo.wTableID;
	}
	uint16 CRoomUserItem::GetLastTableID()
	{
		return m_UserInfo.wLastTableID;
	}
	uint16 CRoomUserItem::GetChairID()
	{
		return m_UserInfo.wChairID;
	}
	uint8 CRoomUserItem::GetUserStatus()
	{
		return m_UserInfo.cbUserStatus;
	}
	uint64 CRoomUserItem::GetUserScore()
	{
		return m_UserInfo.lScore;
	}
	uint16 CRoomUserItem::GetUserWinRate()
	{
		uint32 dwPlayCount = GetUserPlayCount();
		if (dwPlayCount != 0) return (uint16)(m_UserInfo.dwWinCount * 10000 / dwPlayCount);

		return 100;
	}
	uint16 CRoomUserItem::GetUserLostRate()
	{
		uint32 dwPlayCount = GetUserPlayCount();
		if (dwPlayCount != 0) return (uint16)(m_UserInfo.dwLostCount * 10000 / dwPlayCount);

		return 100;
	}
	uint16 CRoomUserItem::GetUserDrawRate()
	{
		uint32 dwPlayCount = GetUserPlayCount();
		if (dwPlayCount != 0) return (uint16)(m_UserInfo.dwDrawCount * 10000 / dwPlayCount);

		return 100;
	}
	uint16 CRoomUserItem::GetUserFleeRate()
	{
		uint32 dwPlayCount = GetUserPlayCount();
		if (dwPlayCount != 0) return (uint16)(m_UserInfo.dwFleeCount * 10000 / dwPlayCount);

		return 100;
	}
	uint16 CRoomUserItem::GetUserPlayCount()
	{
		return m_UserInfo.dwWinCount + m_UserInfo.dwLostCount + m_UserInfo.dwDrawCount + m_UserInfo.dwFleeCount;
	}

	bool CRoomUserItem::ContrastLogonPass(const char * szPassword)
	{
		return strcmp(m_szLogonPass, szPassword) == 0;
	}

	bool CRoomUserItem::IsTrusteeUser()
	{
		return m_bTrusteeUser;
	}
	void CRoomUserItem::SetTrusteeUser(bool bTrusteeUser)
	{
		m_bTrusteeUser = bTrusteeUser;
	}
	bool CRoomUserItem::IsClientReady()
	{
		return m_bClientReady;
	}
	void CRoomUserItem::SetClientReady(bool bClientReady)
	{
		m_bClientReady = bClientReady;
	}
	bool CRoomUserItem::SetUserStatus(uint8 cbUserStatus, uint16 wTableID, uint16 wChairID)
	{
		//效验状态
		if (m_UserInfo.dwUserID == 0) return false;

		//记录信息
		uint16 wOldTableID = m_UserInfo.wTableID;
		uint16 wOldChairID = m_UserInfo.wChairID;

		//设置变量
		m_UserInfo.wTableID = wTableID;
		m_UserInfo.wChairID = wChairID;
		m_UserInfo.cbUserStatus = cbUserStatus;
		if (cbUserStatus == US_PLAYING)m_UserInfo.wLastTableID = wTableID;

		//发送状态
		assert(m_pIRoomUserItemSink != nullptr);
		if (m_pIRoomUserItemSink != nullptr) m_pIRoomUserItemSink->OnEventUserItemStatus(this, wOldTableID, wOldChairID);
	}

	bool CRoomUserItem::SetUserParameter(uint32 dwClientAddr, uint16 wBindIndex, const char szMachineID[LEN_MACHINE_ID], bool bClientReady)
	{
		assert(m_UserInfo.dwUserID != 0);
		if (m_UserInfo.dwUserID == 0) return false;

		m_bClientReady = bClientReady;
		m_wBindIndex = wBindIndex;
		m_dwClientAddr = dwClientAddr;
		sprintf_s(m_szMachineID, szMachineID, sizeof(m_szMachineID));
		return true;
	}

	void CRoomUserItem::ResetUserItem()
	{
	}

	/////////////////////////////////////////////////////////////
	CRoomUserManager::CRoomUserManager()
	{
	}
	CRoomUserManager::~CRoomUserManager()
	{
	}
	void * CRoomUserManager::QueryInterface(GGUID uuid)
	{
		QUERY_INTERFACE(IRoomUserManager, uuid);
		QUERY_INTERFACE_IUNKNOWNEX(IRoomUserManager, uuid);
		return nullptr;
	}
	bool CRoomUserManager::SetServerUserItemSink(IUnknownEx * pIUnknownEx)
	{
		//设置接口
		if (pIUnknownEx != nullptr)
		{
			//查询接口
			assert(QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, IRoomUserItemSink) != nullptr);
			m_pIRoomUserItemSink = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, IRoomUserItemSink);

			//成功判断
			if (m_pIRoomUserItemSink == nullptr) return false;
		}
		else m_pIRoomUserItemSink = nullptr;
		return true;
	}
	IRoomUserItem * CRoomUserManager::SearchUserItem(uint32 dwUserID)
	{
		RUIM_IT it = m_UserItemMap.find(dwUserID);
		if (it != m_UserItemMap.end())
		{
			return it->second;
		}
		return nullptr;
	}
	IRoomUserItem * CRoomUserManager::SearchUserItem(char * pszNickName)
	{
		for (RUIM_IT it = m_UserItemMap.begin(); it != m_UserItemMap.end(); ++it)
		{
			if (strcmp(it->second->GetNickName(), pszNickName) == 0)
			{
				return it->second;
			}
		}
		return nullptr;
	}

	CRoomUserManager::CRoomUserItemMap & CRoomUserManager::TraverseRoomUserList()
	{
		return m_UserItemMap;
	}
	uint32 CRoomUserManager::GetAndroidCount()
	{
		return uint32();
	}
	uint32 CRoomUserManager::GetUserItemCount()
	{
		return uint32();
	}
	bool CRoomUserManager::DeleteUserItem()
	{
		return false;
	}
	bool CRoomUserManager::DeleteUserItem(IRoomUserItem * pIServerUserItem)
	{
		return false;
	}
	bool CRoomUserManager::InsertUserItem(IRoomUserItem ** pIServerUserResult, tagUserInfo & UserInfo, tagUserInfoPlus &UserInfoPlus)
	{
		//变量定义
		CRoomUserItem * pServerUserItem = nullptr;
		if (m_FreeUserItem.empty())
		{
			try
			{
				pServerUserItem = new CRoomUserItem;
			}
			catch (...)
			{
				assert(nullptr);
				return false;
			}
		}
		else
		{
			pServerUserItem = m_FreeUserItem.back();
			m_FreeUserItem.pop_back();

			pServerUserItem->ResetUserItem();
		}

		//设置接口
		pServerUserItem->m_pIRoomUserItemSink = m_pIRoomUserItemSink;
		memcpy(&pServerUserItem->m_UserInfo, &UserInfo, sizeof(UserInfo));
		
		//连接信息
		pServerUserItem->m_wBindIndex = UserInfoPlus.wBindIndex;
		pServerUserItem->m_dwClientAddr = UserInfoPlus.dwClientAddr;
		sprintf_s(pServerUserItem->m_szMachineID, "%s", UserInfoPlus.szMachineID);

		//辅助变量
		pServerUserItem->m_bClientReady = false;
		pServerUserItem->m_bTrusteeUser = false;
		
		m_UserItemMap[UserInfo.dwUserID] = pServerUserItem;

		//设置变量
		*pIServerUserResult = pServerUserItem;

		return true;
	}
}