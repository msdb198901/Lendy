#ifndef SERVICE_UNITS_H
#define SERVICE_UNITS_H


#include "KernelEngineHead.h"
#include "DBExports.h"
#include "IOContext.h"
#include "AttemperEngineSink.h"
#include "Header.h"
#include "Strand.h"
#include "DataQueue.h"
#include <functional>
#include <thread>
#include <mutex>

namespace Game
{
	class ServiceUnits
	{
		//服务状态
		enum enServiceStatus
		{
			ServiceStatus_Stop,				//停止状态
			ServiceStatus_Config,			//配置状态
			ServiceStatus_Run,				//服务状态
		};

	public:
		ServiceUnits();

	public:
		static ServiceUnits* GetInstance();

	public:
		//启动服务
		bool Start(Net::IOContext* ioContext);

		//停止服务
		bool Conclude();

		//内部运行
	protected:
		void Run();

		//内部函数
	protected:
		//配置组件
		bool InitializeService();
		//启动内核
		bool StartKernelService(Net::IOContext*);

		//内部函数
	private:
		//设置状态
		bool SetServiceStatus(enServiceStatus ServiceStatus);
		//发送控制
		bool SendControlPacket(uint16 wControlID, void * pData, uint16 wDataSize);

	public:
		//投递请求
		bool PostControlRequest(uint16 wIdentifier, void * pData, uint16 wDataSize);

	protected:
		//控制消息
		bool OnUIControlRequest();

		//仿函数
	private:
		std::function<bool()>				m_funcStartNetService;

	private:
		//状态变量
		enServiceStatus						m_ServiceStatus;

		//服务组件
	public:
		CAttemperEngineSink					m_AttemperEngineSink;				//调度钩子

	private:
		CAttemperEngineHelper				m_AttemperEngine;					//调度引擎
		CTCPNetworkEngineHelper				m_TCPNetworkEngine;
		CTCPSocketServiceHelper				m_TCPSocketService;					//服务通讯

		//内部綫程
	private:
		std::mutex							m_mutex;
		Net::IOContext 						m_ioContext;
		Net::Strand*						m_pStrand;
		std::thread*						m_pThread;
		DataQueue							m_dataQueue;
	};
}

#define SrvUnitsMgr Game::ServiceUnits::GetInstance()

#endif