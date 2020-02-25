#include "AttemperEngine.h"

namespace Net
{
	CAttemperEngine::CAttemperEngine():
		m_ioContext(nullptr),
		m_pStrand(nullptr),
		m_pITCPNetworkEngine(nullptr),
		m_pIAttemperEngineSink(nullptr)
	{
	}

	CAttemperEngine::~CAttemperEngine()
	{
	}

	bool CAttemperEngine::Start(Net::IOContext* ioContext)
	{
		if (ioContext)
		{
			m_ioContext = ioContext;
			m_pStrand = new Net::Strand(*ioContext);
		}

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

	bool CAttemperEngine::Stop()
	{
		PDELETE(m_pStrand);

		//效验参数
		assert(m_pIAttemperEngineSink != nullptr);
		if (m_pIAttemperEngineSink == nullptr) return false;

		//启动通知
		if (!m_pIAttemperEngineSink->OnAttemperEngineConclude(QUERY_ME_INTERFACE(IUnknownEx)))
		{
			assert(nullptr);
			return false;
		}
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
		if (m_ioContext)
		{
			Net::post(*m_ioContext, Net::bind_executor(*m_pStrand, [this, wControlID, pData, wDataSize]() {m_pIAttemperEngineSink->OnEventControl(wControlID, pData, wDataSize); }));
		}
		return true;
	}

	bool CAttemperEngine::OnEventTCPNetworkBind(uint64 dwSocketID, uint64 dwClientAddr)
	{
		if (m_ioContext)
		{
			Net::post(*m_ioContext, Net::bind_executor(*m_pStrand, [this, dwClientAddr, dwSocketID]() {m_pIAttemperEngineSink->OnEventTCPNetworkBind(dwClientAddr, dwSocketID); }));
		}
		return true;
	}

	bool CAttemperEngine::OnEventTCPNetworkShut(uint64 dwSocketID, uint64 dwClientAddr)
	{
		if (m_ioContext)
		{
			Net::post(*m_ioContext, Net::bind_executor(*m_pStrand, [this, dwClientAddr, dwSocketID]() { m_pIAttemperEngineSink->OnEventTCPNetworkShut(dwClientAddr, dwSocketID); }));
		}
		return true;
	}
	bool CAttemperEngine::OnEventTCPNetworkRead(uint64 dwSocketID, Net::TCP_Command Command, void * pData, uint16 wDataSize)
	{
		if (m_ioContext)
		{
			Net::post(*m_ioContext, Net::bind_executor(*m_pStrand, [this, dwSocketID, Command, pData, wDataSize]() { m_pIAttemperEngineSink->OnEventTCPNetworkRead(Command, pData, wDataSize, dwSocketID);}));
		}
		return true;
	}

	DECLARE_CREATE_MOUDLE(AttemperEngine);
}
	 


