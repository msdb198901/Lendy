#ifndef ROOM_USER_MANAGER_H
#define ROOM_USER_MANAGER_H

#include "Struct.h"
#include "GameComponent.h"
#include <map>

namespace Game
{
	using namespace Comm;
	class CRoomUserItem : public IRoomUserItem
	{
		//友元定义
		friend class CRoomUserManager;

		//函数定义
	protected:
		//构造函数
		CRoomUserItem();
		//析构函数
		virtual ~CRoomUserItem();

		//基础接口
	public:
		//释放对象
		virtual void Release() { delete this; };
		//接口查询
		virtual void *QueryInterface(GGUID uuid);

		//属性信息
	public:
		//用户索引
		virtual uint16 GetBindIndex();
		//用户地址
		virtual uint64 GetClientAddr();
		//机器标识
		virtual char* GetMachineID();

		//登录信息
	public:
		//请求标识
		virtual uint64 GetDBQuestID();
		//登录时间
		virtual uint64 GetLogonTime();
		//记录索引
		virtual uint64 GetInoutIndex();

		//用户信息
	public:
		//用户信息
		virtual tagUserInfo * GetUserInfo();

		//属性信息
	public:
		//用户性别
		virtual uint8 GetGender();
		//用户标识
		virtual uint32 GetUserID();
		//游戏标识
		virtual uint32 GetGameID();
		//用户昵称
		virtual char* GetNickName();

		//状态接口
	public:
		//桌子号码
		virtual uint16 GetTableID();
		//桌子号码
		virtual uint16 GetLastTableID();
		//椅子号码
		virtual uint16 GetChairID();
		//用户状态
		virtual uint8 GetUserStatus();
		//解除绑定
		virtual bool DetachBindStatus();

		//积分信息
	public:
		//用户积分
		virtual uint64 GetUserScore();

		//积分信息
	public:
		//用户胜率
		virtual uint16 GetUserWinRate();
		//用户输率
		virtual uint16 GetUserLostRate();
		//用户和率
		virtual uint16 GetUserDrawRate();
		//用户逃率
		virtual uint16 GetUserFleeRate();
		//游戏局数
		virtual uint16 GetUserPlayCount();
		
		//效验接口
	public:
		//对比密码
		virtual bool ContrastLogonPass(const char* szPassword);

		//托管状态
	public:
		//判断状态
		virtual bool IsTrusteeUser();
		//设置状态
		virtual void SetTrusteeUser(bool bTrusteeUser);

		//游戏状态
	public:
		//连接状态
		virtual bool IsClientReady();
		//设置连接
		virtual void SetClientReady(bool bClientReady);

		//管理接口
	public:
		//设置状态
		virtual bool SetUserStatus(uint8 cbUserStatus, uint16 wTableID, uint16 wChairID);

		//高级接口
	public:
		//设置参数
		virtual bool SetUserParameter(uint32 dwClientAddr, uint16 wBindIndex, const char szMachineID[LEN_MACHINE_ID], bool bClientReady);

		//辅助函数
	private:
		//重置数据
		void ResetUserItem();

		//属性变量
	protected:
		tagUserInfo						m_UserInfo;							//用户信息

		//组件接口
	protected:
		IRoomUserItemSink *				m_pIRoomUserItemSink;				//回调接口

		//辅助变量
	protected:
		bool							m_bClientReady;
		bool							m_bTrusteeUser;						//系统托管
		char							m_szLogonPass[LEN_PASSWORD];		//用户密码

		//系统属性
	protected:
		uint16							m_wBindIndex;						//绑定索引
		uint32							m_dwClientAddr;						//连接地址
		char							m_szMachineID[LEN_MACHINE_ID];		//机器标识
	};

	class CRoomUserManager : public IRoomUserManager
	{
		
		typedef std::vector<CRoomUserItem*>			CRoomUserItemArray;

	public:
		typedef std::map<uint32, CRoomUserItem*>	CRoomUserItemMap;
		typedef CRoomUserItemMap::iterator			RUIM_IT;
		
		//函数定义
	public:
		//构造函数
		CRoomUserManager();
		//析构函数
		virtual ~CRoomUserManager();

		//基础接口
	public:
		//释放对象
		virtual void Release() { delete this; };
		//接口查询
		virtual void *QueryInterface(GGUID uuid);

		//配置接口
	public:
		//设置接口
		virtual bool SetServerUserItemSink(IUnknownEx * pIUnknownEx);

		//查找接口
	public:
		//查找用户
		virtual IRoomUserItem * SearchUserItem(uint32 dwUserID);
		//查找用户
		virtual IRoomUserItem * SearchUserItem(char* pszNickName);

		//遍历接口
	public:
		CRoomUserItemMap &TraverseRoomUserList();

		//统计接口
	public:
		//机器人数
		virtual uint32 GetAndroidCount();
		//在线人数
		virtual uint32 GetUserItemCount();

		//管理接口
	public:
		//删除用户
		virtual bool DeleteUserItem();
		//删除用户
		virtual bool DeleteUserItem(IRoomUserItem * pIServerUserItem);
		//插入用户
		virtual bool InsertUserItem(IRoomUserItem * * pIServerUserResult, tagUserInfo & UserInfo, tagUserInfoPlus &UserInfoPlus);

		//组件接口
	protected:
		IRoomUserItemSink *							m_pIRoomUserItemSink;				//回调接口

		//用户变量
	protected:
		CRoomUserItemMap							m_UserItemMap;
		CRoomUserItemArray							m_FreeUserItem;
	};
}

#endif