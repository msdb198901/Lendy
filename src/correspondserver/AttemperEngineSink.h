#ifndef ATTEMPER_ENGINE_SINK_H
#define ATTEMPER_ENGINE_SINK_H

#include "KernelEngineHead.h"
#include "DBExports.h"
#include "GlobalInfoManager.h"

namespace Correspond
{
	using namespace Net;

	//服务类型
	enum enServiceKind
	{
		ServiceKind_None,				//无效服务
		ServiceKind_Game,				//游戏服务
		ServiceKind_Plaza,				//广场服务
		ServiceKind_Chat,				//好友服务
	};

	//绑定参数
	struct tagBindParameter
	{
		//网络参数
		uint32							dwSocketID;							//网络标识
		uint32							dwClientAddr;						//连接地址
																	
		uint16							wServiceID;							//服务标识
		enServiceKind					ServiceKind;						//服务类型
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
		virtual bool OnEventTCPNetworkRead(TCP_Command Command, void * pData, uint16 wDataSize, uint32 dwSocketID);

		//接口事件
	public:
		//控制事件
		virtual bool OnEventControl(uint16 wControlID, void * pData, uint16 wDataSize);

		//内核事件
	public:
		//时间事件
		virtual bool OnEventTimer(uint32 dwTimerID) { return true; }
		
		//网络事件
	protected:
		//注册服务
		bool OnTCPNetworkMainRegister(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);
		//服务状态
		bool OnTCPNetworkMainServiceInfo(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);
		//用户汇总
		bool OnTCPNetworkMainUserCollect(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);
		//远程服务
		bool OnTCPNetworkMainRemoteService(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);
		//管理服务
		bool OnTCPNetworkMainManagerService(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);
		//机器服务
		bool OnTCPNetworkMainAndroidService(uint16 wSubCmdID, void * pData, uint16 wDataSize, uint32 dwSocketID);

		//辅助函数
	protected:
		//发送列表
		bool SendRoomListToLogon(uint32 dwSocketID);


		//状态变量
	protected:
		uint16							m_wCollectItem;						//汇总连接
		std::vector<uint16>				m_WaitCollectItemArray;				//汇总等待

	private:
		tagBindParameter *				m_pBindParameter;					//辅助数组

		//组件接口
	protected:
		ITCPNetworkEngine *				m_pITCPNetworkEngine;				//网络引擎

		//组件变量
	protected:
		CGlobalInfoManager				m_GlobalInfoManager;				//全局管理
	};
}

#endif