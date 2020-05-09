#ifndef ASYNCHRONISM_ENGINE_H
#define ASYNCHRONISM_ENGINE_H

#include "../KernelEngineHead.h"
#include "Strand.h"
#include "DataQueue.h"
#include <thread>
#include <mutex>

namespace Net
{
	class CAsynchronismEngine : public IAsynchronismEngine
	{
		//函数定义
	public:
		//构造函数
		CAsynchronismEngine();
		//析构函数
		virtual ~CAsynchronismEngine();

		//基础接口
	public:
		//释放对象
		virtual void Release() { delete this; }
		//接口查询
		virtual void *QueryInterface(GGUID uuid);

		//服务接口
	public:
		//启动服务
		virtual bool Start(Net::IOContext*);
		//停止服务
		virtual bool Stop();

		//异步接口
	public:
		//设置模块
		virtual bool SetAsynchronismSink(IUnknownEx * pIUnknownEx);
		//异步数据
		virtual bool PostAsynchronismData(uint16 wIdentifier, void * pData, uint16 wDataSize);

	protected:
		void Run();

	protected:
		IAsynchronismEngineSink *			m_pIAsynchronismEngineSink;			//回调接口

		std::shared_ptr<Net::IOContext> 	m_ioContext;
		Net::Strand*						m_pStrand;
		std::thread*						m_pThread;

		//辅助变量
	private:
		std::mutex							m_mutex;
		uint8								m_cbBuffer[SOCKET_TCP_BUFFER];	//接收缓冲
		Util::DataQueue						m_dataQueue;
	};
}

#endif