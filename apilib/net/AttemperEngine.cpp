#include "AttemperEngine.h"

namespace Net
{
	CAttemperEngine::CAttemperEngine():
		m_pITCPNetworkEngine(nullptr),
		m_pIAttemperEngineSink(nullptr)
	{
		memset(m_cbBuffer, 0, sizeof(m_cbBuffer));
	}

	CAttemperEngine::~CAttemperEngine()
	{
	}

	bool CAttemperEngine::Start(Net::IOContext* ioContext)
	{
		//效验参数
		assert(m_pIAttemperEngineSink != nullptr);
		if (m_pIAttemperEngineSink == nullptr) return false;

		//注册对象
		IUnknownEx * pIAsynchronismEngineSink = QUERY_ME_INTERFACE(IUnknownEx);
		if (m_AsynchronismEngine.SetAsynchronismSink(pIAsynchronismEngineSink) == false)
		{
			assert(nullptr);
			return false;
		}

		//异步引擎
		if (!m_AsynchronismEngine.Start(ioContext))
		{
			assert(nullptr);
			return false;
		}
		////启动通知
		//if (!m_pIAttemperEngineSink->OnAttemperEngineStart(QUERY_ME_INTERFACE(IUnknownEx)))
		//{
		//	assert(nullptr);
		//	return false;
		//}
		return true;
	}

	bool CAttemperEngine::Stop()
	{
		m_AsynchronismEngine.Stop();

		////效验参数
		//assert(m_pIAttemperEngineSink != nullptr);
		//if (m_pIAttemperEngineSink == nullptr) return false;

		//启动通知
		//if (!m_pIAttemperEngineSink->OnAttemperEngineConclude(QUERY_ME_INTERFACE(IUnknownEx)))
		//{
		//	assert(nullptr);
		//	return false;
		//}
		return true;
	}

	void CAttemperEngine::Release()
	{
		delete this;
	}

	void * CAttemperEngine::QueryInterface(GGUID uuid)
	{
		QUERY_INTERFACE(IAttemperEngine, uuid);
		QUERY_INTERFACE(ITCPNetworkEngineEvent, uuid);
		QUERY_INTERFACE(IAsynchronismEngineSink, uuid);
		QUERY_INTERFACE_IUNKNOWNEX(IAttemperEngine, uuid);
		return nullptr;
	}

	bool CAttemperEngine::SetNetworkEngine(IUnknownEx * pIUnknownEx)
	{
		//设置接口
		if (pIUnknownEx != nullptr)
		{
			//查询接口
			assert(QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, ITCPNetworkEngine) != nullptr);
			m_pITCPNetworkEngine = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, ITCPNetworkEngine);

			//成功判断
			if (m_pITCPNetworkEngine == nullptr) return false;
		}
		else m_pITCPNetworkEngine = nullptr;

		return true;
	}

	bool CAttemperEngine::SetAttemperEngineSink(IUnknownEx * pIUnknownEx)
	{
		//查询接口
		assert(QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, IAttemperEngineSink) != NULL);
		m_pIAttemperEngineSink = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, IAttemperEngineSink);

		//结果判断
		if (m_pIAttemperEngineSink == nullptr)
		{
			assert(FALSE);
			return false;
		}
		return true;
	}

	bool CAttemperEngine::OnEventControl(uint16 wControlID, void * pData, uint16 wDataSize)
	{
		//if (m_ioContext)
		//{
		//	Net::post(*m_ioContext, Net::bind_executor(*m_pStrand, [this, wControlID, pData, wDataSize]() {m_pIAttemperEngineSink->OnEventControl(wControlID, pData, wDataSize); }));
		//}
		return true;
	}

	bool CAttemperEngine::OnEventTCPNetworkBind(uint64 dwSocketID, uint64 dwClientAddr)
	{
		//缓冲锁定
		std::lock_guard<std::mutex> _lock(m_mutex);
		AS_TCPNetworkAcceptEvent * pAcceptEvent = (AS_TCPNetworkAcceptEvent *)m_cbBuffer;

		//构造数据
		pAcceptEvent->dwSocketID = dwSocketID;
		pAcceptEvent->dwClientAddr = dwClientAddr;

		//投递数据
		return m_AsynchronismEngine.PostAsynchronismData(EVENT_TCP_CLIENT_ACCEPT, m_cbBuffer, sizeof(AS_TCPNetworkAcceptEvent));
	}

	bool CAttemperEngine::OnEventTCPNetworkShut(uint64 dwSocketID, uint64 dwClientAddr)
	{
		//缓冲锁定
		std::lock_guard<std::mutex> _lock(m_mutex);
		AS_TCPNetworkShutEvent * pCloseEvent = (AS_TCPNetworkShutEvent *)m_cbBuffer;

		//构造数据
		pCloseEvent->dwSocketID = dwSocketID;
		pCloseEvent->dwClientAddr = dwClientAddr;

		//投递数据
		return m_AsynchronismEngine.PostAsynchronismData(EVENT_TCP_CLIENT_SHUT, m_cbBuffer, sizeof(AS_TCPNetworkShutEvent));
	}
	bool CAttemperEngine::OnEventTCPNetworkRead(uint64 dwSocketID, Net::TCP_Command Command, void * pData, uint16 wDataSize)
	{
		assert((wDataSize + sizeof(AS_TCPNetworkReadEvent)) <= SOCKET_TCP_BUFFER);
		if ((wDataSize + sizeof(AS_TCPNetworkReadEvent)) > SOCKET_TCP_BUFFER) return false;

		std::lock_guard<std::mutex> _lock(m_mutex);
		AS_TCPNetworkReadEvent* pReadEvent = (AS_TCPNetworkReadEvent *)m_cbBuffer;

		//构造数据
		pReadEvent->Command = Command;
		pReadEvent->wDataSize = wDataSize;
		pReadEvent->dwSocketID = dwSocketID;

		//附加数据
		if (wDataSize > 0)
		{
			assert(pData != NULL);
			memcpy(m_cbBuffer + sizeof(AS_TCPNetworkReadEvent), pData, wDataSize);
		}

		return m_AsynchronismEngine.PostAsynchronismData(EVENT_TCP_CLIENT_READ, m_cbBuffer, sizeof(AS_TCPNetworkReadEvent) + wDataSize); 
	}

	bool CAttemperEngine::OnAsynchronismEngineStart()
	{
		//效验参数
		assert(m_pIAttemperEngineSink != nullptr);
		if (m_pIAttemperEngineSink == nullptr) return false;

		//启动通知
		if (!m_pIAttemperEngineSink->OnAttemperEngineStart(QUERY_ME_INTERFACE(IUnknownEx)))
		{
			assert(nullptr);
			return false;
		}
		return true;
	}

	bool CAttemperEngine::OnAsynchronismEngineConclude()
	{
		//效验参数
		assert(m_pIAttemperEngineSink != nullptr);
		if (m_pIAttemperEngineSink == nullptr) return false;

		//停止通知
		if (!m_pIAttemperEngineSink->OnAttemperEngineConclude(QUERY_ME_INTERFACE(IUnknownEx)))
		{
			assert(nullptr);
			return false;
		}
		return true;
	}

	bool CAttemperEngine::OnAsynchronismEngineData(uint16 wIdentifier, void * pData, uint16 wDataSize)
	{
		//效验参数
		assert(m_pITCPNetworkEngine != NULL);
		assert(m_pIAttemperEngineSink != NULL);

		//内核事件
		switch (wIdentifier)
		{
			case EVENT_TCP_CLIENT_ACCEPT:
			{
				//大小断言
				assert(wDataSize == sizeof(AS_TCPNetworkAcceptEvent));
				if (wDataSize != sizeof(AS_TCPNetworkAcceptEvent)) return false;

				//变量定义
				bool bSuccess = false;
				AS_TCPNetworkAcceptEvent * pAcceptEvent = (AS_TCPNetworkAcceptEvent *)pData;

				//处理消息
				try
				{
					bSuccess = m_pIAttemperEngineSink->OnEventTCPNetworkBind(pAcceptEvent->dwClientAddr, pAcceptEvent->dwSocketID);
				}
				catch (...) {}

				//失败处理
				if (bSuccess == false) m_pITCPNetworkEngine->CloseSocket(pAcceptEvent->dwSocketID);

				return true;
			}
			case EVENT_TCP_CLIENT_READ:		//读取事件
			{
				//效验大小
				AS_TCPNetworkReadEvent * pReadEvent = (AS_TCPNetworkReadEvent *)pData;

				//大小断言
				assert(wDataSize >= sizeof(AS_TCPNetworkReadEvent));
				assert(wDataSize == (sizeof(AS_TCPNetworkReadEvent) + pReadEvent->wDataSize));

				//大小效验
				if (wDataSize < sizeof(AS_TCPNetworkReadEvent))
				{
					m_pITCPNetworkEngine->CloseSocket(pReadEvent->dwSocketID);
					return false;
				}

				//大小效验
				if (wDataSize != (sizeof(AS_TCPNetworkReadEvent) + pReadEvent->wDataSize))
				{
					m_pITCPNetworkEngine->CloseSocket(pReadEvent->dwSocketID);
					return false;
				}

				//处理消息
				bool bSuccess = m_pIAttemperEngineSink->OnEventTCPNetworkRead(pReadEvent->Command, pReadEvent + 1, pReadEvent->wDataSize, pReadEvent->dwSocketID);
				
				//失败处理
				if (!bSuccess) m_pITCPNetworkEngine->CloseSocket(pReadEvent->dwSocketID);
				
				return true;
			}
		}

		return false;
	}

	DECLARE_CREATE_MODULE(AttemperEngine);
}
	 


