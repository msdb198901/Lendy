#include "TCPNetworkEngine.h"


namespace Net
{
	CTCPNetworkEngine::CTCPNetworkEngine() :
		m_curIndex(0),
		m_strBindIP(""),
		m_port(0),
		m_acceptor(nullptr),
		m_threadCount(0),
		m_pThreads(nullptr),
		m_pITCPNetworkEngineEvent(nullptr)
	{}

	CTCPNetworkEngine::~CTCPNetworkEngine()
	{
	}

	void CTCPNetworkEngine::Release()
	{
		delete this;
	}

	void * CTCPNetworkEngine::QueryInterface(GGUID uuid)
	{
		QUERY_INTERFACE(IServiceMoudle, uuid);
		QUERY_INTERFACE(ITCPNetworkEngine, uuid);
		QUERY_INTERFACE_IUNKNOWNEX(ITCPNetworkEngine, uuid);
		return nullptr;
	}

	bool CTCPNetworkEngine::SetTCPNetworkEngineEvent(IUnknownEx * pIUnknownEx)
	{
		assert(m_threadCount == 0);
		if (m_threadCount > 0) return false;

		//查询接口
		m_pITCPNetworkEngineEvent = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, ITCPNetworkEngineEvent);

		//错误判断
		if (m_pITCPNetworkEngineEvent == nullptr)
		{
			assert(FALSE);
			return false;
		}
		return true;
	}

	bool CTCPNetworkEngine::SetServiceParameter(std::string strBindIP, uint16 port, uint16 threadCount)
	{
		assert(threadCount > 0);
		if (threadCount == 0) return false;

		m_strBindIP = std::move(strBindIP);
		m_port = port;
		m_threadCount = threadCount;
		return true;
	}

	bool CTCPNetworkEngine::OnEventSocketBind(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem)
	{
		//效验数据
		assert(pTCPNetworkItem != nullptr);
		assert(m_pITCPNetworkEngineEvent != nullptr);

		m_pITCPNetworkEngineEvent->OnEventTCPNetworkBind(pTCPNetworkItem->GetIndex(), pTCPNetworkItem->GetClientIP());
		return true;
	}

	bool CTCPNetworkEngine::OnEventSocketShut(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem)
	{
		//效验数据
		assert(pTCPNetworkItem != nullptr);
		assert(m_pITCPNetworkEngineEvent != nullptr);

		m_pITCPNetworkEngineEvent->OnEventTCPNetworkShut(pTCPNetworkItem->GetIndex(), pTCPNetworkItem->GetClientIP());
		
		FreeNetworkItem(pTCPNetworkItem);
		return true;
	}

	bool CTCPNetworkEngine::OnEventSocketRead(TCP_Command Command, void * pData, uint16 wDataSize, std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem)
	{
		//效验数据
		assert(pTCPNetworkItem != nullptr);
		assert(m_pITCPNetworkEngineEvent != nullptr);

		m_pITCPNetworkEngineEvent->OnEventTCPNetworkRead(pTCPNetworkItem->GetIndex(), Command, pData, wDataSize);
		return true;
	}

	bool CTCPNetworkEngine::SendData(uint64 dwSocketID, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		
		return false;
	}

	bool CTCPNetworkEngine::Start(Net::IOContext* ioContext)
	{
		assert(m_threadCount > 0);
		if (m_threadCount == 0) return false;

		AsyncAcceptor *acceptor = nullptr;
		try
		{
			acceptor = new AsyncAcceptor(*ioContext, m_strBindIP, m_port);
		}
		catch (asio::error_code const&)
		{
			assert(nullptr);
			return false;
		}

		if (!acceptor->Bind())
		{
			assert(nullptr);
			delete acceptor;
			return false;
		}

		m_acceptor = acceptor;
		m_pThreads = CreateThreads();

		for (int i = 0; i < 1; ++i)
		{
			m_pThreads[i].Start();
		}

		m_acceptor->AsyncAcceptWithCallBack(std::bind(&CTCPNetworkEngine::OnSocketOpen, this, std::placeholders::_1, std::placeholders::_2));

		return true;
	}

	bool CTCPNetworkEngine::Stop()
	{
		m_acceptor->Close();

		for (int i = 0; i < m_threadCount; ++i)
		{
			m_pThreads[i].Stop();
		}

		Wait();

		PDELETE(m_acceptor);
		ADELETE(m_pThreads);
		m_threadCount = 0;

		return true;
	}

	void CTCPNetworkEngine::OnSocketOpen(tcp::socket && _socket, uint32 threadIndex)
	{
		try
		{
			std::shared_ptr<CTCPNetworkItem> newSocket = ActiveNetworkItem(std::move(_socket));
			m_pThreads[threadIndex].AddSocket(newSocket);
			newSocket->Start();
		}
		catch (asio::error_code const&)
		{
			assert(nullptr);
		}
	}

	int CTCPNetworkEngine::GetNetworkThreadCount() const
	{
		return m_threadCount;
	}

	uint32 CTCPNetworkEngine::SelectThreadWithMinConnections() const
	{
		uint32 min = 0;
		for (int i = 1; i < m_threadCount; ++i)
		{
			if (m_pThreads[i].GetConnectionCount() < m_pThreads[0].GetConnectionCount())
			{
				min = i;
			}
		}
		return min;
	}

	std::pair<tcp::socket*, uint32> CTCPNetworkEngine::GetAcceptSocket()
	{
		uint32 threadIndex = SelectThreadWithMinConnections();
		return std::make_pair(m_pThreads[threadIndex].GetAcceptSocket(), threadIndex);
	}

	void CTCPNetworkEngine::Wait()
	{
		for (int i = 0; i < m_threadCount; ++i)
		{
			m_pThreads[i].Wait();
		}
	}

	std::shared_ptr<CTCPNetworkItem> CTCPNetworkEngine::ActiveNetworkItem(tcp::socket && _socket)
	{
		//TODO...暂不判断当前连接数

		//获取对象
		std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem = nullptr;
		if (!m_NetworkFreeItem.empty())
		{
			pTCPNetworkItem = m_NetworkFreeItem.front();
			m_NetworkFreeItem.pop_front();
			pTCPNetworkItem->Attach(std::move(_socket));
		}

		//创建对象
		if (pTCPNetworkItem == nullptr)
		{
			pTCPNetworkItem = std::make_shared<CTCPNetworkItem>(m_curIndex++, std::move(_socket), this);
			//pTCPNetworkItem = std::make_shared<CTCPNetworkItem>(m_curIndex++, std::move(_socket), this
			//		std::bind(&CTCPNetworkEngine::OnEventSocketBind, this, std::placeholders::_1),
			//		std::bind(&CTCPNetworkEngine::OnEventSocketShut, this, std::placeholders::_1),
			//		std::bind(&CTCPNetworkEngine::OnEventSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		}
		return pTCPNetworkItem;
	}

	std::shared_ptr<CTCPNetworkItem> CTCPNetworkEngine::GetNetworkItem(uint16 wIndex)
	{
		return nullptr;
	}

	bool CTCPNetworkEngine::FreeNetworkItem(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem)
	{
		m_NetworkFreeItem.push_back(pTCPNetworkItem);
		return true;
	}

	CTCPNetworkThread<CTCPNetworkItem>* CTCPNetworkEngine::CreateThreads()
	{
		return new CTCPNetworkThread<CTCPNetworkItem>[1];
	}

	DECLARE_CREATE_MOUDLE(TCPNetworkEngine);
}