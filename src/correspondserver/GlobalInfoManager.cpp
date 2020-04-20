#include "GlobalInfoManager.h"

namespace Correspond
{
	bool CGlobalInfoManager::DeleteLogonItem(uint16 wPlazaID)
	{
		AGLC_IT it = m_ActiveGameLogon.find(wPlazaID);
		if (it == m_ActiveGameLogon.end())
		{
			assert(nullptr);
			return false;
		}
		m_FreeGameLogon.emplace_back(it->second);
		m_ActiveGameLogon.erase(it);
		return true;
	}

	bool CGlobalInfoManager::ActiveLogonItem(tagGameLogon & GamePlaza)
	{
		if (m_ActiveGameLogon.find(GamePlaza.wPlazaID) != m_ActiveGameLogon.end())
		{
			assert(nullptr);
			return false;
		}
		tagGameLogon* pGameLogon = CreateGlobalLogonItem();
		if (pGameLogon == nullptr)
		{
			assert(nullptr);
			return false;
		}
		memcpy(pGameLogon, &GamePlaza, sizeof(tagGameLogon));
		m_ActiveGameLogon[GamePlaza.wPlazaID] = pGameLogon;
		return true;
	}

	bool CGlobalInfoManager::DeleteRoomItem(uint16 wServerID)
	{
		AGRC_IT it = m_ActiveGameRoom.find(wServerID);
		if (it == m_ActiveGameRoom.end())
		{
			assert(nullptr);
			return false;
		}
		m_FreeGameRoom.emplace_back(it->second);
		m_ActiveGameRoom.erase(it);
		return true;
	}

	bool CGlobalInfoManager::ActiveRoomItem(tagGameRoom & GameRoom)
	{
		if (m_ActiveGameRoom.find(GameRoom.wServerID) != m_ActiveGameRoom.end())
		{
			assert(nullptr);
			return false;
		}
		tagGameRoom* pGameRoom = CreateGlobalRoomItem();
		if (pGameRoom == nullptr)
		{
			assert(nullptr);
			return false;
		}
		memcpy(pGameRoom, &GameRoom, sizeof(tagGameRoom));
		m_ActiveGameRoom[GameRoom.wServerID] = pGameRoom;
		return true;
	}

	CGlobalInfoManager::ActiveGameRoomContainer & CGlobalInfoManager::TraverseGameRoom()
	{
		return m_ActiveGameRoom;
	}

	tagGameRoom * CGlobalInfoManager::SearchRoomItem(uint16 wServerID)
	{
		AGRC_IT it = m_ActiveGameRoom.find(wServerID);
		if (it == m_ActiveGameRoom.end())
		{
			return nullptr;
		}
		return it->second;
	}

	tagGameLogon * CGlobalInfoManager::CreateGlobalLogonItem()
	{
		if (m_FreeGameLogon.empty())
		{
			//创建对象
			try
			{
				tagGameLogon * pGlobalLogonItem = new tagGameLogon;
				return pGlobalLogonItem;
			}
			catch (...) {}
		}
		else
		{
			tagGameLogon * pGlobalLogonItem = m_FreeGameLogon.back();
			m_FreeGameLogon.pop_back();
			return pGlobalLogonItem;
		}
		return nullptr;
	}
	tagGameRoom * CGlobalInfoManager::CreateGlobalRoomItem()
	{
		if (m_FreeGameRoom.empty())
		{
			//创建对象
			try
			{
				tagGameRoom * pGlobalRoomItem = new tagGameRoom;
				return pGlobalRoomItem;
			}
			catch (...) {}
		}
		else
		{
			tagGameRoom * pGlobalRoomItem = m_FreeGameRoom.back();
			m_FreeGameRoom.pop_back();
			return pGlobalRoomItem;
		}
		return nullptr;
	}
}