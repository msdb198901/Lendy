#ifndef ROOM_LIST_MANAGER_H
#define ROOM_LIST_MANAGER_H

#include "Struct.h"
#include <map>
#include <vector>

namespace Logon
{
	using namespace Comm;
	typedef std::vector<tagGameRoom*>			FreeRoomItemContainer;
	typedef std::map<uint16, tagGameRoom*>		RoomItemMap;
	typedef RoomItemMap::iterator				RIM_IT;

	typedef std::map<uint16, tagGameKind*>		KindItemMap;
	typedef KindItemMap::iterator				KIM_IT;

	class CRoomListManager
	{
	protected:
		RoomItemMap					m_RoomList;
		KindItemMap					m_KindList;

		FreeRoomItemContainer		m_FreeRoomItemArray;

		//函数定义
	public:
		//构造函数
		CRoomListManager();
		//析构函数
		virtual ~CRoomListManager();

	public:
		//废弃房间
		void DisuseRoomItem();

		//查找接口
	public:
		//查找种类
		tagGameKind * SearchGameKind(uint16 wKindID);

		//查找房间
		tagGameRoom * SearchRoomServer(uint16 wServerID);

		//插入接口
	public:
		//插入房间
		bool InsertGameServer(tagGameRoom * pGameRoom);

	};
}

#endif