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

	typedef std::vector<tagGameKind*>			FreeKindItemContainer;
	typedef std::map<uint16, tagGameKind*>		KindItemMap;
	typedef KindItemMap::iterator				KIM_IT;

	class CRoomListManager
	{
	protected:
		RoomItemMap					m_RoomList;
		KindItemMap					m_KindList;

		FreeRoomItemContainer		m_FreeRoomItemArray;
		FreeKindItemContainer		m_FreeKindItemArray;

		//函数定义
	public:
		//构造函数
		CRoomListManager();
		//析构函数
		virtual ~CRoomListManager();

	public:
		//废弃房间
		void DisuseRoomItem();

		//遍历接口
	public:
		RoomItemMap &TraverseRoomList();

		KindItemMap &TraverseKindList();

		//查找接口
	public:
		//查找种类
		tagGameKind * SearchGameKind(uint16 wKindID);

		//查找房间
		tagGameRoom * SearchGameRoom(uint16 wServerID);

		//插入接口
	public:
		//插入房间
		bool InsertGameRoom(tagGameRoom * pGameRoom);
		//插入房间
		bool InsertGameKind(tagGameKind * pGameKind);

		//删除接口
	public:
		//删除房间
		bool DeleteGameRoom(uint16 wServerID);
	};
}

#endif