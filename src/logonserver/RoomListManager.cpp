#include "RoomListManager.h"

namespace Logon
{
	CRoomListManager::CRoomListManager()
	{
	}

	CRoomListManager::~CRoomListManager()
	{
	}

	void CRoomListManager::DisuseRoomItem()
	{
	}

	tagGameKind * CRoomListManager::SearchGameKind(uint16 wKindID)
	{
		KIM_IT it = m_KindList.find(wKindID);
		if (it != m_KindList.end())
		{
			return it->second;
		}
		return nullptr;
	}

	tagGameRoom * CRoomListManager::SearchRoomServer(uint16 wServerID)
	{
		RIM_IT it = m_RoomList.find(wServerID);
		if (it != m_RoomList.end())
		{
			return it->second;
		}
		return nullptr;
	}

	bool CRoomListManager::InsertGameServer(tagGameRoom * pGameRoom)
	{
		assert(pGameRoom != nullptr);
		if (pGameRoom == nullptr) return false;

		uint16 wOldKindID = 0;
		uint32 dwOldOnLineCount = 0;
		uint32 dwOldAndroidCount = 0;
		uint32 dwOldMaxPlayer = 0;

		tagGameRoom * pGameRoomItem = SearchRoomServer(pGameRoom->wServerID);
		if (pGameRoomItem == nullptr)
		{
			try
			{
				if (!m_FreeRoomItemArray.empty())
				{
					pGameRoomItem = m_FreeRoomItemArray.back();
					m_FreeRoomItemArray.pop_back();
				}
				else
				{
					pGameRoomItem = new tagGameRoom;
					if (pGameRoomItem == nullptr) return false;
				}
			}
			catch (...)
			{
				return false;
			}
			memset(pGameRoomItem, 0, sizeof(tagGameRoom));
		}
		else
		{
			wOldKindID			= pGameRoomItem->wKindID;
			dwOldOnLineCount	= pGameRoomItem->dwOnLineCount;
			dwOldAndroidCount	= pGameRoomItem->dwAndroidCount;
			dwOldMaxPlayer		= pGameRoomItem->dwFullCount;
		}
		memcpy(pGameRoomItem, pGameRoom, sizeof(tagGameRoom));

		if (wOldKindID != pGameRoomItem->wKindID)
		{
			KIM_IT itOldKind = m_KindList.find(wOldKindID);
			if (itOldKind != m_KindList.end())
			{
				itOldKind->second->dwOnLineCount -= dwOldOnLineCount;
				itOldKind->second->dwAndroidCount -= dwOldAndroidCount;
				itOldKind->second->dwFullCount -= dwOldMaxPlayer;
			}

			KIM_IT itNewKind = m_KindList.find(pGameRoomItem->wKindID);
			if (itNewKind != m_KindList.end())
			{
				itNewKind->second->dwOnLineCount += pGameRoomItem->dwOnLineCount;
				itNewKind->second->dwAndroidCount += pGameRoomItem->dwAndroidCount;
				itNewKind->second->dwFullCount += pGameRoomItem->dwFullCount;
			}
		}
		else
		{
			KIM_IT itKind = m_KindList.find(pGameRoomItem->wKindID);
			if (itKind != m_KindList.end())
			{
				itKind->second->dwOnLineCount -= dwOldOnLineCount;
				itKind->second->dwAndroidCount -= dwOldAndroidCount;
				itKind->second->dwFullCount -= dwOldMaxPlayer;

				itKind->second->dwOnLineCount += pGameRoomItem->dwOnLineCount;
				itKind->second->dwAndroidCount += pGameRoomItem->dwAndroidCount;
				itKind->second->dwFullCount += pGameRoomItem->dwFullCount;
			}
		}

		m_RoomList[pGameRoomItem->wServerID] = pGameRoomItem;
		return true;
	}
}