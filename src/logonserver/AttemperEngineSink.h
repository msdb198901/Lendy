#ifndef ATTEMPER_ENGINE_SINK_H
#define ATTEMPER_ENGINE_SINK_H

#include "KernelEngineHead.h"
#include "DBExports.h"
#include "Header.h"
#include "RoomListManager.h"

namespace Logon
{
	//绑定参数
	struct tagBindParameter
	{
		//网络参数
		uint64							dwSocketID;							//网络标识
		uint64							dwClientAddr;						//连接地址
		uint8							cbClientKind;						//连接类型
	};

	enum LinkType
	{
		LT_FLASH	= 1,				//网页类型
		LT_MOBILE	= 2,				//手机类型
		LT_COMPUTER = 3,				//电脑类型
	};

	class CAttemperEngineSink : public IAttemperEngineSink
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
		//时间事件
		virtual bool OnEventTimer(uint32 dwTimerID);
		
		//连接处理
	protected:
		//注册事件
		bool OnTCPSocketMainRegister(uint16 wSubCmdID, void * pData, uint16 wDataSize);
		//列表事件
		bool OnTCPSocketMainServiceInfo(uint16 wSubCmdID, void * pData, uint16 wDataSize);

		//手机事件
	protected:
		//登录处理
		bool OnTCPNetworkMainMBLogon(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);

	protected:
		//游客登录
		bool OnTCPNetworkSubMBLogonVisitor(void * pData, uint16 wDataSize, uint32 dwSocketID);

		//手机列表
	protected:
		//发送房间
		void SendKindListInfo(uint32 dwSocketID);
		//发送房间
		void SendRoomListInfo(uint32 dwSocketID, uint16 wKindID);

	protected:
		//登陆失败
		bool OnLogonFailure(uint32 dwSocketID, LogonErrorCode &lec);

	private:
		tagBindParameter *				m_pBindParameter;					//辅助数组

		//组件接口
	protected:
		ITimerEngine *					m_pITimerEngine;					//时间引擎
		ITCPNetworkEngine *				m_pITCPNetworkEngine;				//网络引擎
		ITCPSocketService *				m_pITCPSocketService;

		//组件变量
	protected:
		CRoomListManager				m_RoomListManager;					//列表管理

	};
}

#endif