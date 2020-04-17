#include "AsynchronismEngine.h"

#include "ProducerConsumerQueue.h"

namespace Net
{
	CAsynchronismEngine::CAsynchronismEngine() :
		m_ioContext(std::make_shared<Net::IOContext>()),
		m_pStrand(new Net::Strand(*m_ioContext)),
		m_pThread(nullptr)
	{
	}

	CAsynchronismEngine::~CAsynchronismEngine()
	{
		Stop();
	}

	void * CAsynchronismEngine::QueryInterface(GGUID uuid)
	{
		QUERY_INTERFACE(IServiceModule, uuid);
		QUERY_INTERFACE(IAsynchronismEngine, uuid);
		QUERY_INTERFACE_IUNKNOWNEX(IAsynchronismEngine, uuid);
		return nullptr;
	}

	bool CAsynchronismEngine::Start(Net::IOContext*)
	{
		if (m_pThread = new std::thread(&CAsynchronismEngine::Run, this), m_pThread == nullptr)
		{
			assert(nullptr);
			return false;
		}
		return true;
	}

	bool CAsynchronismEngine::Stop()
	{
		m_ioContext->stop();
		if (m_pThread)
		{
			m_pThread->join();
			PDELETE(m_pThread);
		}
		PDELETE(m_pStrand);
		return true;
	}

	bool CAsynchronismEngine::SetAsynchronismSink(IUnknownEx * pIUnknownEx)
	{
		//运行判断
		assert(m_pThread == nullptr);
		if (m_pThread) return false;

		//查询接口
		m_pIAsynchronismEngineSink = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, IAsynchronismEngineSink);

		//错误判断
		if (m_pIAsynchronismEngineSink == NULL)
		{
			assert(nullptr);
			return false;
		}
		return true;
	}

	bool CAsynchronismEngine::PostAsynchronismData(uint16 wIdentifier, void * pData, uint16 wDataSize)
	{
		if (m_ioContext)
		{
			std::lock_guard<std::mutex>	 _lock(m_mutex);
			if (!m_dataQueue.InsertData(wIdentifier, pData, wDataSize))
			{
				assert(nullptr);
				return false;
			}

			Net::post(*m_ioContext, Net::bind_executor(*m_pStrand, [this, wIdentifier, pData, wDataSize]() 
			{
				//提取数据
				m_mutex.lock();
				Util::tagDataHead DataHead;
				m_dataQueue.DistillData(DataHead, m_cbBuffer, sizeof(m_cbBuffer));
				m_mutex.unlock();

				m_pIAsynchronismEngineSink->OnAsynchronismEngineData(DataHead.wIdentifier, m_cbBuffer, DataHead.wDataSize);
			}));
		}
		return true;
	}

	void CAsynchronismEngine::Run()
	{
		//事件通知
		assert(m_pIAsynchronismEngineSink != NULL);
		bool bSuccess = m_pIAsynchronismEngineSink->OnAsynchronismEngineStart();

		asio::io_context::work work(*m_ioContext);
		m_ioContext->run();

		m_pIAsynchronismEngineSink->OnAsynchronismEngineConclude();
	}

	DECLARE_CREATE_MODULE(AsynchronismEngine);
}