#ifndef GAME_SERVER_MANAGER_H
#define GAME_SERVER_MANAGER_H

#include "../gameserver/GameComponent.h"

namespace SubGame
{
	using namespace Game;

	class CGameServerManager : public IGameServiceManager
	{
		//函数定义
	public:
		//构造函数
		CGameServerManager(void);
		//析构函数
		virtual ~CGameServerManager(void);

		static CGameServerManager* GetInstance();

		//基础接口
	public:
		//释放对象
		virtual void Release() { }
		//接口查询
		virtual void * QueryInterface(GGUID Guid);

		//创建接口
	public:
		//创建桌子
		virtual void * CreateTableFrameSink(GGUID Guid);
		//创建机器
		virtual void * CreateAndroidUserItemSink(GGUID Guid);
		//创建数据
		virtual void * CreateGameDataBaseEngineSink(GGUID Guid);

		//参数接口
	public:
		//调整参数
		virtual bool RectifyParameter(tagGameServiceOption & GameServiceOption);
	};
}

#define sGSMgr SubGame::CGameServerManager::GetInstance()

#endif