#include "RoomListManager.h"

namespace Logon
{
	CRoomListManager::CRoomListManager()
	{
	}

	CRoomListManager::~CRoomListManager()
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

	tagGameServer * CRoomListManager::SearchRoomServer(uint16 wServerID)
	{
		RIM_IT it = m_RoomList.find(wServerID);
		if (it != m_RoomList.end())
		{
			return it->second;
		}
		return nullptr;
	}
}