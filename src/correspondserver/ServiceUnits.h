#ifndef SERVICE_UNITS_H
#define SERVICE_UNITS_H


#include "KernelEngineHead.h"
#include "IOContext.h"
#include "Header.h"
#include "AttemperEngineSink.h"

namespace Correspond
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

	private:
		//状态变量
		enServiceStatus						m_ServiceStatus;

		//服务组件
	public:
		CAttemperEngineSink					m_AttemperEngineSink;				//调度钩子

	private:
		CAttemperEngineHelper				m_AttemperEngine;					//调度引擎
		CTCPNetworkEngineHelper				m_TCPNetworkEngine;
	};
}

#define SrvUnitsMgr Correspond::ServiceUnits::GetInstance()

#endif