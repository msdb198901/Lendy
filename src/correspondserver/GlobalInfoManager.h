#ifndef GLOBAL_INFO_MANAGER_H
#define GLOBAL_INFO_MANAGER_H

#include "Define.h"
#include "Struct.h"
#include <map>
#include <vector>

namespace Correspond
{
	using namespace Comm;

	//全局信息
	class CGlobalInfoManager
	{
		typedef std::map<uint16, tagGameLogon*>	ActiveGameLogonContainer;
		typedef std::map<uint16, tagGameLogon*>::iterator	AGLC_IT;
		typedef std::vector<tagGameLogon*>		FreeGameLogonContainer;

	public:
		typedef std::map<uint16, tagGameRoom*>	ActiveGameRoomContainer;
		typedef std::map<uint16, tagGameRoom*>::iterator	AGRC_IT;
		typedef std::vector<tagGameRoom*>		FreeGameRoomContainer;

		//广场管理
	public:
		//删除广场
		bool DeleteLogonItem(uint16 wPlazaID);
		//激活广场
		bool ActiveLogonItem(uint16 wBindIndex,tagGameLogon &GamePlaza);

		//房间管理
	public:
		//删除房间
		bool DeleteRoomItem(uint16 wServerID);
		//激活房间
		bool ActiveRoomItem(uint16 wBindIndex, tagGameRoom & GameServer);
		//遍历房间
		ActiveGameRoomContainer &TraverseGameRoom();
		//寻找房间
		tagGameRoom * SearchRoomItem(uint16 wServerID);

	private:
		tagGameLogon * CreateGlobalLogonItem();
		tagGameRoom * CreateGlobalRoomItem();

	protected:
		ActiveGameLogonContainer			m_ActiveGameLogon;
		FreeGameLogonContainer				m_FreeGameLogon;

		ActiveGameRoomContainer				m_ActiveGameRoom;
		FreeGameRoomContainer				m_FreeGameRoom;
	};

}

#endif