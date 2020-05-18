#include "GlobalInfoManager.h"
#include <algorithm>
#include <cassert>
#include <cstring>

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

		//删除用户
		for (AUC_IT it_auc = m_ActiveUserItem.begin(); it_auc != m_ActiveUserItem.end(); )
		{
			for (G_IT it_g = it_auc->second->vGameRoomID.begin(); it_g != it_auc->second->vGameRoomID.end(); )
			{
				if (*it_g == wServerID)
				{
					it_g = it_auc->second->vGameRoomID.erase(it_g);
				}
				else
				{
					++it_g;
				}
			}
			if (it_auc->second->vGameRoomID.empty())
			{
				m_ActiveUserItem.erase(it_auc++);
			}
			else
			{
				++it_auc;
			}
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
		tagGlobalRoomItem* pGlobalRoomItem = CreateGlobalRoomItem();
		if (pGlobalRoomItem == nullptr)
		{
			assert(nullptr);
			return false;
		}
		memcpy(&pGlobalRoomItem->gGameRoom, &GameRoom, sizeof(tagGameRoom));
		m_ActiveGameRoom[GameRoom.wServerID] = pGlobalRoomItem;
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
		return &it->second->gGameRoom;
	}

	bool CGlobalInfoManager::DeleteUserItem(uint32 dwUserID, uint16 wServerID)
	{
		tagGlobalUserItem *pGlobalUserItem = nullptr;
		AUC_IT it = m_ActiveUserItem.find(dwUserID);
		if (it == m_ActiveUserItem.end())
		{
			assert(false);
			return false;
		}

		for (G_IT it_g = it->second->vGameRoomID.begin(); it_g != it->second->vGameRoomID.end();)
		{
			uint16  dwRemoveRoomID = *it_g;
			if (dwRemoveRoomID == wServerID)
			{
				AGRC_IT it_agrc = m_ActiveGameRoom.find(dwRemoveRoomID);
				if (it_agrc == m_ActiveGameRoom.end())
				{
					assert(nullptr);
					return false;
				}
				it_agrc->second->vUserID.erase(std::remove(it_agrc->second->vUserID.begin(), it_agrc->second->vUserID.end(), dwUserID), it_agrc->second->vUserID.end());
				it_g = it->second->vGameRoomID.erase(it_g);
			}
			else
			{
				++it_g;
			}
		}

		if (it->second->vGameRoomID.empty())
		{
			FreeGlobalUserItem(it->second);
		}
		return true;
	}

	bool CGlobalInfoManager::ActiveUserItem(tagGlobalUserItem & GlobalUserInfo, uint16 wServerID)
	{
		tagGlobalRoomItem *pGlobalRoomItem = nullptr;
		AGRC_IT it_agrc = m_ActiveGameRoom.find(wServerID);
		if (it_agrc == m_ActiveGameRoom.end())
		{
			assert(nullptr);
			return false;
		}

		tagGlobalUserItem *pGlobalUserItem = nullptr;
		AUC_IT it_auc = m_ActiveUserItem.find(GlobalUserInfo.gUserInfo.dwUserID);
		if (it_auc == m_ActiveUserItem.end())
		{
			pGlobalUserItem = CreateGlobalUserItem();
			if (pGlobalUserItem == nullptr)
			{
				assert(nullptr);
				return false;
			}
			memcpy(&pGlobalUserItem->gUserInfo, &GlobalUserInfo.gUserInfo, sizeof(pGlobalUserItem->gUserInfo));
			m_ActiveUserItem[GlobalUserInfo.gUserInfo.dwUserID] = pGlobalUserItem;
		}
		else
		{
			for (G_IT it = it_auc->second->vGameRoomID.begin(); it != it_auc->second->vGameRoomID.end(); ++it)
			{
				if (*it == wServerID)
				{
					assert(nullptr);
					return false;
				}
			}
			pGlobalUserItem = it_auc->second;
		}

		pGlobalUserItem->vGameRoomID.emplace_back(wServerID);
		for (U_IT it = it_agrc->second->vUserID.begin(); it != it_agrc->second->vUserID.end(); ++it)
		{
			if (*it == GlobalUserInfo.gUserInfo.dwUserID)
			{
				return true;
			}
		}
		it_agrc->second->vUserID.emplace_back(GlobalUserInfo.gUserInfo.dwUserID);
		return true;
	}

	tagGlobalUserItem * CGlobalInfoManager::SearchUserItemByUserID(uint32 dwUserID)
	{
		AUC_IT it = m_ActiveUserItem.find(dwUserID);
		if (it != m_ActiveUserItem.end())
		{
			return it->second;
		}
		return nullptr;
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
	tagGlobalRoomItem * CGlobalInfoManager::CreateGlobalRoomItem()
	{
		if (m_FreeGameRoom.empty())
		{
			//创建对象
			try
			{
				tagGlobalRoomItem * pGlobalRoomItem = new tagGlobalRoomItem;
				return pGlobalRoomItem;
			}
			catch (...) {}
		}
		else
		{
			tagGlobalRoomItem * pGlobalRoomItem = m_FreeGameRoom.back();
			m_FreeGameRoom.pop_back();
			return pGlobalRoomItem;
		}
		return nullptr;
	}
	tagGlobalUserItem * CGlobalInfoManager::CreateGlobalUserItem()
	{
		if (m_FreeUserItem.empty())
		{
			//创建对象
			try
			{
				tagGlobalUserItem * pGlobalUserItem = new tagGlobalUserItem;
				return pGlobalUserItem;
			}
			catch (...) {}
		}
		else
		{
			tagGlobalUserItem * pGlobalUserItem = m_FreeUserItem.back();
			m_FreeUserItem.pop_back();
			return pGlobalUserItem;
		}
		return nullptr;
	}
	bool CGlobalInfoManager::FreeGlobalUserItem(tagGlobalUserItem * pGlobalUserItem)
	{
		//效验参数
		assert(pGlobalUserItem != nullptr);
		if (pGlobalUserItem == nullptr) return false;

		m_FreeUserItem.emplace_back(pGlobalUserItem);
		return true;
	}
}