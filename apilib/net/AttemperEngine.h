#ifndef ATTEMPER_ENGINE_H
#define ATTEMPER_ENGINE_H

#include "../KernelEngineHead.h"
#include "AsynchronismEngine.h"
#include <mutex>

namespace Net
{
	class CAttemperEngine : public IAttemperEngine, public ITCPNetworkEngineEvent, public IAsynchronismEngineSink
	{
		//函数定义
public:
		//构造函数
		CAttemperEngine();
		//析构函数
		virtual ~CAttemperEngine();

		//服务接口
	public:
		//启动服务
		virtual bool Start(Net::IOContext* ioContext);
		//停止服务
		virtual bool Stop();

		virtual void Release();
		virtual void *QueryInterface(GGUID uuid);

		//配置接口
	public:
		//网络接口
		virtual bool SetNetworkEngine(IUnknownEx * pIUnknownEx);
		//回调接口
		virtual bool SetAttemperEngineSink(IUnknownEx * pIUnknownEx);

		//控制事件
	public:
		//控制事件
		virtual bool OnEventControl(uint16 wControlID, void * pData, uint16 wDataSize);

	public:
		//应答事件
		virtual bool OnEventTCPNetworkBind(uint64 dwSocketID, uint64 dwClientAddr);
		//关闭事件
		virtual bool OnEventTCPNetworkShut(uint64 dwSocketID, uint64 dwClientAddr);
		//读取事件
		virtual bool OnEventTCPNetworkRead(uint64 dwSocketID, Net::TCP_Command Command, void * pData, uint16 wDataSize);

		//异步接口
	public:
		//启动事件
		virtual bool OnAsynchronismEngineStart();
		//停止事件
		virtual bool OnAsynchronismEngineConclude();
		//异步数据
		virtual bool OnAsynchronismEngineData(uint16 wIdentifier, void * pData, uint16 wDataSize);

	private:
		ITCPNetworkEngine *			m_pITCPNetworkEngine;				//网络接口
		IAttemperEngineSink*		m_pIAttemperEngineSink;

		//组件变量
	protected:
		std::mutex					m_mutex;
		CAsynchronismEngine			m_AsynchronismEngine;				//异步引擎

		//辅助变量
	protected:
		BYTE						m_cbBuffer[SOCKET_TCP_BUFFER];		//临时对象
	};
}

#endif