#ifndef GLOBAL_INFO_MANAGER_H
#define GLOBAL_INFO_MANAGER_H

#include "Define.h"
#include "Struct.h"
#include <map>
#include <vector>

namespace Correspond
{
	using namespace Comm;

	typedef std::vector<uint16>				GameRoomIDArray;
	typedef GameRoomIDArray::iterator		G_IT;
	typedef std::vector<uint32>				UserIDArray;
	typedef UserIDArray::iterator			U_IT;

	//用户信息
	struct tagGlobalUserItem
	{
		tagUserInfo						gUserInfo;
		GameRoomIDArray					vGameRoomID;		

		//更新状态
		void UpdateStatus(const uint16 wTableID, const uint16 wChairID, const uint8 cbUserStatus)
		{
			gUserInfo.wTableID = wTableID;
			gUserInfo.wChairID = wChairID;
			
			gUserInfo.cbUserStatus = cbUserStatus;
		}
	};

	struct tagGlobalRoomItem
	{
		tagGameRoom						gGameRoom;
		UserIDArray						vUserID;
	};

	//全局信息
	class CGlobalInfoManager
	{
		typedef std::map<uint16, tagGameLogon*>	ActiveGameLogonContainer;
		typedef std::map<uint16, tagGameLogon*>::iterator	AGLC_IT;
		typedef std::vector<tagGameLogon*>		FreeGameLogonContainer;

	public:
		typedef std::map<uint16, tagGlobalRoomItem*>			ActiveGameRoomContainer;
		typedef std::map<uint16, tagGlobalRoomItem*>::iterator	AGRC_IT;
		typedef std::vector<tagGlobalRoomItem*>					FreeGameRoomContainer;

		typedef std::map<uint32, tagGlobalUserItem*>			ActiveUserContainer;
		typedef std::map<uint32, tagGlobalUserItem*>::iterator	AUC_IT;
		typedef std::vector<tagGlobalUserItem*>					FreeUserContainer;

		//广场管理
	public:
		//删除广场
		bool DeleteLogonItem(uint16 wPlazaID);
		//激活广场
		bool ActiveLogonItem(tagGameLogon &GamePlaza);

		//房间管理
	public:
		//删除房间
		bool DeleteRoomItem(uint16 wServerID);
		//激活房间
		bool ActiveRoomItem(tagGameRoom & GameServer);
		//遍历房间
		ActiveGameRoomContainer &TraverseGameRoom();
		//寻找房间
		tagGameRoom * SearchRoomItem(uint16 wServerID);

		//用户管理
	public:
		//删除用户
		bool DeleteUserItem(uint32 dwUserID, uint16 wServerID);
		//激活用户
		bool ActiveUserItem(tagGlobalUserItem &GlobalUserInfo, uint16 wServerID);
		
		//用户查找
	public:
		//寻找用户
		tagGlobalUserItem * SearchUserItemByUserID(uint32 dwUserID);

	private:
		tagGameLogon * CreateGlobalLogonItem();
		tagGlobalRoomItem * CreateGlobalRoomItem();
		tagGlobalUserItem * CreateGlobalUserItem();

		//释放函数
	private:
		//释放用户
		bool FreeGlobalUserItem(tagGlobalUserItem * pGlobalUserItem);

	protected:
		ActiveGameLogonContainer			m_ActiveGameLogon;
		FreeGameLogonContainer				m_FreeGameLogon;

		ActiveGameRoomContainer				m_ActiveGameRoom;
		FreeGameRoomContainer				m_FreeGameRoom;

		ActiveUserContainer					m_ActiveUserItem;
		FreeUserContainer					m_FreeUserItem;
	};

}

#endif