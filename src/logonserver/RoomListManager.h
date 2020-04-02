#ifndef ROOM_LIST_MANAGER_H
#define ROOM_LIST_MANAGER_H

#include "Struct.h"
#include <map>

namespace Logon
{
	using namespace comm;
	typedef std::map<uint16, tagGameServer*>	RoomItemMap;
	typedef RoomItemMap::iterator				RIM_IT;

	typedef std::map<uint16, tagGameKind*>		KindItemMap;
	typedef KindItemMap::iterator				KIM_IT;

	class CRoomListManager
	{
	protected:
		RoomItemMap			m_RoomList;
		KindItemMap			m_KindList;

		//函数定义
	public:
		//构造函数
		CRoomListManager();
		//析构函数
		virtual ~CRoomListManager();

		//查找接口
	public:
		//查找种类
		tagGameKind * SearchGameKind(uint16 wKindID);

		//查找房间
		tagGameServer * SearchRoomServer(uint16 wServerID);

		

	};
}

#endif