#ifndef ATTEMPER_ENGINE_SINK_H
#define ATTEMPER_ENGINE_SINK_H

#include "DBExports.h"
#include "Header.h"
#include "TableFrame.h"
#include "GameComponent.h"
#include "RoomUserManager.h"

namespace Game
{
	//绑定参数
	struct tagBindParameter
	{
		//网络参数
		uint32							dwSocketID;							//网络标识
		uint32							dwClientAddr;						//连接地址
		uint8							cbClientKind;						//连接类型
		IRoomUserItem *				pIServerUserItem;
	};

	enum LinkType
	{
		LT_FLASH	= 1,				//网页类型
		LT_MOBILE	= 2,				//手机类型
		LT_COMPUTER = 3,				//电脑类型
	};

	typedef std::vector<CTableFrame *> CTableFrameArray;

	class CAttemperEngineSink : public IAttemperEngineSink , public IMainServiceFrame, public IRoomUserItemSink
	{
		friend class ServiceUnits;

	public:
		CAttemperEngineSink();
		virtual ~CAttemperEngineSink();

		virtual void Release();
		virtual void *QueryInterface(GGUID uuid);

		//异步接口
	public:
		//启动事件
		virtual bool OnAttemperEngineStart(IUnknownEx * pIUnknownEx);
		//停止事件
		virtual bool OnAttemperEngineConclude(IUnknownEx * pIUnknownEx);

		//连接事件
	public:
		//连接事件
		virtual bool OnEventTCPSocketLink(uint16 wServiceID, int iErrorCode);
		//关闭事件
		virtual bool OnEventTCPSocketShut(uint16 wServiceID, uint8 cbShutReason);
		//读取事件
		virtual bool OnEventTCPSocketRead(uint16 wServiceID, TCP_Command Command, void * pData, uint16 wDataSize);

		//网络事件
	public:
		//应答事件
		virtual bool OnEventTCPNetworkBind(uint32 dwClientAddr, uint32 dwSocketID);
		//关闭事件
		virtual bool OnEventTCPNetworkShut(uint32 dwClientAddr, uint32 dwSocketID);
		//读取事件
		virtual bool OnEventTCPNetworkRead(Net::TCP_Command Command, void * pData, uint16 wDataSize, uint32 dwSocketID);

		//接口事件
	public:
		//控制事件
		virtual bool OnEventControl(uint16 wControlID, void * pData, uint16 wDataSize);
		
		//内核事件
	public:
		virtual bool OnEventTimer(uint32 dwTimerID);

		//消息接口
	public:
		//房间消息
		virtual bool SendRoomMessage(char* lpszMessage, uint16 wType);
		//游戏消息
		virtual bool SendGameMessage(char* lpszMessage, uint16 wType);
		//房间消息
		virtual bool SendRoomMessage(IRoomUserItem * pIServerUserItem, const char* lpszMessage, uint16 wType);
		//游戏消息
		virtual bool SendGameMessage(IRoomUserItem * pIServerUserItem, const char* lpszMessage, uint16 wType);
		//房间消息
		virtual bool SendRoomMessage(uint32 dwSocketID, const char* lpszMessage, uint16 wType, bool bAndroid);

		//发送数据
		virtual bool SendData(uint32 dwSocketID, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize);
		//发送数据
		virtual bool SendData(IRoomUserItem * pIServerUserItem, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize);
		//群发数据
		virtual bool SendDataBatch(uint16 wCmdTable, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize);
		//解锁积分
		virtual void UnLockScoreLockUser(uint32 dwUserID, uint32 dwInoutIndex, uint32 dwLeaveReason);

		//用户接口
	public:
		//用户状态
		virtual bool OnEventUserItemStatus(IRoomUserItem * pIServerUserItem, uint16 wOldTableID = INVALID_TABLE, uint16 wOldChairID = INVALID_CHAIR);

		//发送函数
	protected:
		//用户信息
		bool SendUserInfoPacket(IRoomUserItem * pIServerUserItem, uint32 dwSocketID);
		//群发用户信息
		bool SendUserInfoPacketBatch(IRoomUserItem * pIServerUserItem, uint32 dwSocketID);

		//辅助函数
	protected:
		//配置桌子
		bool InitTableFrameArray();

		//连接处理
	protected:
		//注册事件
		bool OnTCPSocketMainRegister(uint16 wSubCmdID, void * pData, uint16 wDataSize);
		//列表事件
		bool OnTCPSocketMainServiceInfo(uint16 wSubCmdID, void * pData, uint16 wDataSize);
		
		//网络事件
	protected:
		//用户处理
		bool OnTCPNetworkMainUser(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);
		//登录处理
		bool OnTCPNetworkMainLogon(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);
		//框架处理
		bool OnTCPNetworkMainFrame(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);

	protected:
		//游客登录
		bool OnTCPNetworkSubMBLogonVisitor(void * pData, uint16 wDataSize, uint32 dwSocketID);

		//用户命令
	protected:
		//用户坐下
		bool OnTCPNetworkSubUserSitDown(void * pData, uint16 wDataSize, uint32 dwSocketID);

		//执行功能
	protected:
		//切换连接
		bool SwitchUserItemConnect(IRoomUserItem * pIServerUserItem, const char szMachineID[LEN_MACHINE_ID], uint16 wTargetIndex);

		//内部事件
	protected:
		//用户登录
		void OnEventUserLogon(IRoomUserItem * pIServerUserItem, bool bOnLine);
		//用户登出
		void OnEventUserLogout(IRoomUserItem * pIServerUserItem, uint32 dwLeaveReason);

		//辅助函数
	public:
		//绑定用户
		IRoomUserItem  * GetBindUserItem(uint16 wBindIndex);
		//绑定参数
		tagBindParameter * GetBindParameter(uint16 wBindIndex);

		//辅助函数
	protected:
		//登录失败
		bool SendLogonFailure(const char* pszString, uint32 lErrorCode, uint32 dwSocketID);
		//用户失败
		bool SendUserFailure(IRoomUserItem * pIServerUserItem, const char* pszDescribe, uint32 lErrorCode = 0);

		//辅助函数
protected:
		//路单时间
		bool QuerryGameRoomRecordTime(IRoomUserItem * pIServerUserItem);

		//执行功能
protected:
		//解锁游戏
		bool PerformUnlockScore(uint32 dwUserID, uint32 dwInoutIndex, uint32 dwLeaveReason);

		//状态变量
	protected:
		bool							m_bNeekCorrespond;					//协调标志

	private:
		tagBindParameter *				m_pBindParameter;					//辅助数组
		tagGameAddressOption *			m_pGameAddressOption;				//服务地址
		tagGameServiceOption *			m_pGameServiceOption;				//服务配置

		//组件接口
	protected:
		ITimerEngine *					m_pITimerEngine;					//时间引擎
		ITCPNetworkEngine *				m_pITCPNetworkEngine;				//网络引擎
		ITCPSocketService *				m_pITCPSocketService;
		IGameServiceManager *			m_pIGameServiceManager;				//服务管理

		//组件变量
	protected:
		CRoomUserManager				m_ServerUserManager;				//用户管理
		CTableFrameArray				m_TableFrameArray;					//桌子数组
	};
}

#endif