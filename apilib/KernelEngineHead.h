#ifndef KERNEL_ENGINE_HEAD_H
#define KERNEL_ENGINE_HEAD_H

#include <asio/ip/tcp.hpp>
#include "IOContext.h"
#include "Moudle.h"
#include "Packet.h"
//#include "DBEnvHeader.h"

#define NETWORK_ENGINE_DLL_NAME		"Net.dll"
//#define DATABASE_ENGINE_DLL_NAME	"DataBase.dll"

static GGUID IID_IServiceModule = { 0xee6657db, 0x739e, 0x48c4, { 0x96, 0x73, 0x25, 0xce, 0x44, 0xd0, 0xc1, 0xf } };
struct IServiceModule : public IUnknownEx
{
public:
	virtual bool Start(Net::IOContext*) = 0;
	virtual bool Stop() = 0;
};

///////////
namespace Net
{
	static GGUID IID_IAsynchronismEngine = { 0x50312e19, 0x7245, 0x4ea7, { 0xac, 0xb0, 0x2c, 0xe8, 0xe4, 0xfd, 0xcd, 0x9c } };
	static GGUID IID_IAsynchronismEngineSink = { 0x22131182, 0xfdb1, 0x4cfe, { 0xa2, 0xa1, 0x14, 0x19, 0x29, 0x87, 0xca, 0x4b } };
	static GGUID IID_ITCPNetworkEngine = { 0xc876f8aa, 0x199f, 0x48fc, { 0x96, 0x1a, 0x6, 0x67, 0xbc, 0xf3, 0xad, 0xdf } };
	static GGUID IID_IAttemperEngine = { 0x2b972bac, 0x1c76, 0x4e3c, { 0xb9, 0x19, 0x39, 0x43, 0xf4, 0x31, 0x71, 0x83 } };
	static GGUID IID_ITCPNetworkEngineEvent = { 0x507976f6, 0xfc24, 0x4c51, { 0xac, 0xd, 0x60, 0x2f, 0x87, 0x71, 0xfc, 0x83 } };
	static GGUID IID_IAttemperEngineSink = { 0x3620c4d7, 0xfe6, 0x4eee, { 0xad, 0x4f, 0x79, 0xac, 0x23, 0x97, 0xc7, 0x62 } };
	static GGUID IID_ITCPSocketService = { 0x3fbd00dc, 0x72ba, 0x4686, { 0xa4, 0xa1, 0x1e, 0xd2, 0x10, 0x14, 0xbb, 0xd0 } };
	static GGUID IID_ITCPSocketEvent = { 0x82c3bfaa, 0xc4c8, 0x40a8, { 0x9e, 0xd7, 0x15, 0xfb, 0x6d, 0xf2, 0x40, 0xbf } };
	static GGUID IID_ITimerEngine = { 0xc3e77af5, 0x297d, 0x4708, { 0xa2, 0x7b, 0x8f, 0xa8, 0x71, 0xcb, 0x16, 0x59 } };
	static GGUID IID_ITimerEngineEvent = { 0x5c0ff1b6, 0x659d, 0x44fe, { 0x9c, 0xcc, 0x50, 0xc3, 0xf7, 0x86, 0x4a, 0x4d } };

	//////////////////////////////service/////////////////////////////
	//网络接口
	struct ITCPSocketService : public IServiceModule
	{
		//配置接口
	public:
		//配置函数
		virtual bool SetServiceID(uint16 wServiceID) = 0;
		//设置接口
		virtual bool SetTCPSocketEvent(IUnknownEx * pIUnknownEx) = 0;

		//功能接口
	public:
		//关闭连接
		virtual bool CloseSocket() = 0;
		//连接地址
		virtual bool Connect(uint64 dwServerIP, uint16 wPort) = 0;
		//连接地址
		virtual bool Connect(std::string strServerIP, uint16 wPort) = 0;
		//发送函数
		virtual bool SendData(uint16 wMainCmdID, uint16 wSubCmdID, void * pData = nullptr, uint16 wDataSize = 0) = 0;
	};

	//网络事件
	struct ITCPSocketEvent : public IUnknownEx
	{
		//连接事件
		virtual bool OnEventTCPSocketLink(uint16 wServiceID, int nErrorCode) = 0;
		//关闭事件
		virtual bool OnEventTCPSocketShut(uint16 wServiceID, uint8 cbShutReason) = 0;
		//读取事件
		virtual bool OnEventTCPSocketRead(uint16 wServiceID, TCP_Command Command, void * pData, uint16 wDataSize) = 0;
	};

	//////////////////////////////client/////////////////////////////
	//异步引擎
	struct IAsynchronismEngine : public IServiceModule
	{
		//配置接口
	public:
		//设置模块
		virtual bool SetAsynchronismSink(IUnknownEx * pIUnknownEx) = 0;

		//异步数据
		virtual bool PostAsynchronismData(uint16 wIdentifier, void * pData, uint16 wDataSize) = 0;
	};

	//异步钩子
	struct IAsynchronismEngineSink : public IUnknownEx
	{
		//启动事件
		virtual bool OnAsynchronismEngineStart() = 0;
		//停止事件
		virtual bool OnAsynchronismEngineConclude() = 0;
		//异步数据
		virtual bool OnAsynchronismEngineData(uint16 wIdentifier, void * pData, uint16 wDataSize) = 0;
	};

	//调度引擎
	struct IAttemperEngine : public IServiceModule
	{
		//配置接口
	public:
		//网络接口
		virtual bool SetNetworkEngine(IUnknownEx * pIUnknownEx) = 0;
		//回调接口
		virtual bool SetAttemperEngineSink(IUnknownEx * pIUnknownEx) = 0;

		//控制事件
	public:
		//控制事件
		virtual bool OnEventControl(uint16 wControlID, void * pData, uint16 wDataSize) = 0;
	};

	struct ITCPNetworkEngine : public IServiceModule
	{
	public:
		//设置调度
		virtual bool SetTCPNetworkEngineEvent(IUnknownEx * pIUnknownEx) = 0;
		//设置参数
		virtual bool SetServiceParameter(std::string strBindIP, uint16 wServicePort, uint16 threadCount) = 0;

	public:
		//发送函数
		virtual bool SendData(uint64 dwSocketID, uint16 wMainCmdID, uint16 wSubCmdID, void * pData = nullptr, uint16 wDataSize = 0) = 0;
	
		//控制接口
	public:
		//关闭连接
		virtual bool CloseSocket(uint64 dwSocketID) = 0;
	};

	//网络事件
	struct ITCPNetworkEngineEvent : public IServiceModule
	{
		//接口定义
	public:
		//应答事件
		virtual bool OnEventTCPNetworkBind(uint64 dwSocketID, uint64 dwClientAddr) = 0;
		//关闭事件
		virtual bool OnEventTCPNetworkShut(uint64 dwSocketID, uint64 dwClientAddr) = 0;
		//读取事件
		virtual bool OnEventTCPNetworkRead(uint64 dwSocketID, Net::TCP_Command Command, void * pData, uint16 wDataSize) = 0;
	};

	//调度钩子
	struct IAttemperEngineSink : public IUnknownEx
	{
		//异步接口
	public:
		//启动事件
		virtual bool OnAttemperEngineStart(IUnknownEx * pIUnknownEx) = 0;
		//停止事件
		virtual bool OnAttemperEngineConclude(IUnknownEx * pIUnknownEx) = 0;

		//事件接口
	public:
		//控制事件
		virtual bool OnEventControl(uint16 wIdentifier, void * pData, uint16 wDataSize) = 0;

		//内核事件
	public:
		//时间事件
		virtual bool OnEventTimer(uint32 dwTimerID) = 0;

		//连接事件
	public:
		//连接事件
		virtual bool OnEventTCPSocketLink(uint16 wServiceID, int iErrorCode) = 0;
		//关闭事件
		virtual bool OnEventTCPSocketShut(uint16 wServiceID, uint8 cbShutReason) = 0;
		//读取事件
		virtual bool OnEventTCPSocketRead(uint16 wServiceID, TCP_Command Command, void * pData, uint16 wDataSize) = 0;

		//网络事件
	public:
		//应答事件
		virtual bool OnEventTCPNetworkBind(uint64 dwClientAddr, uint64 dwSocketID) = 0;
		//关闭事件
		virtual bool OnEventTCPNetworkShut(uint64 dwClientAddr, uint64 dwSocketID) = 0;
		//读取事件
		virtual bool OnEventTCPNetworkRead(Net::TCP_Command Command, void * pData, uint16 wDataSize, uint64 dwSocketID) = 0;
	};

	//定时器引擎
	struct ITimerEngine : public IServiceModule
	{
		//配置接口
	public:
		//设置接口
		virtual bool SetTimerEngineEvent(IUnknownEx * pIUnknownEx) = 0;

		//功能接口
	public:
		//设置定时器
		virtual bool SetTimer(uint32 dwTimerID, uint32 dwElapse, uint32 dwRepeat) = 0;
		//删除定时器
		virtual bool KillTimer(uint32 dwTimerID) = 0;
		//删除定时器
		virtual bool KillAllTimer() = 0;
	};

	//定时器事件
	struct ITimerEngineEvent : public IUnknownEx
	{
		//接口定义
	public:
		//时间事件
		virtual bool OnEventTimer(uint32 dwTimerID) = 0;
	};

	DECLARE_MOUDLE_HELPER(TimerEngine, NETWORK_ENGINE_DLL_NAME, "CreateTimerEngine")
	DECLARE_MOUDLE_HELPER(AttemperEngine, NETWORK_ENGINE_DLL_NAME, "CreateAttemperEngine")
	DECLARE_MOUDLE_HELPER(TCPNetworkEngine, NETWORK_ENGINE_DLL_NAME, "CreateTCPNetworkEngine")
	DECLARE_MOUDLE_HELPER(TCPSocketService, NETWORK_ENGINE_DLL_NAME, "CreateTCPSocketService")
}

#endif