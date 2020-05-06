#include "TCPNetworkEngine.h"
#include <map>

namespace Net
{
#define ASYNCHRONISM_SEND_DATA		1									//发送标识
#define ASYNCHRONISM_SEND_BATCH		2									//群体发送
#define ASYNCHRONISM_SHUT_DOWN		3									//安全关闭
#define ASYNCHRONISM_ALLOW_BATCH	4									//允许群发
#define ASYNCHRONISM_CLOSE_SOCKET	5									//关闭连接

	//关闭连接
	struct tagCloseSocket
	{
		uint32							dwSocketID;							//连接索引
	};

	//安全关闭
	struct tagShutDownSocket
	{
		uint32							dwSocketID;							//连接索引
	};

	//允许群发
	struct tagAllowBatchSend
	{
		uint32							dwSocketID;							//连接索引
		uint8							cbAllowBatch;						//允许标志
	};

	//群发请求
	struct tagBatchSendRequest
	{
		uint16							wMainCmdID;							//主命令码
		uint16							wSubCmdID;							//子命令码
		uint16							wDataSize;							//数据大小
		uint8							cbSendBuffer[SOCKET_TCP_PACKET];	//发送缓冲
	};

	//发送请求
	struct tagSendData
	{
		uint32							dwSocketID;							//连接索引
		uint16							wMainCmdID;							//主命令码
		uint16							wSubCmdID;							//子命令码
		uint16							wDataSize;							//数据大小
		uint8							cbSendBuffer[SOCKET_TCP_PACKET];	//发送缓冲
	};

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
		QUERY_INTERFACE(IServiceModule, uuid);
		QUERY_INTERFACE(ITCPNetworkEngine, uuid);
		QUERY_INTERFACE(IAsynchronismEngineSink, uuid);
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

	bool CTCPNetworkEngine::OnAsynchronismEngineData(uint16 wIdentifier, void * pData, uint16 wDataSize)
	{
		switch (wIdentifier)
		{
			case ASYNCHRONISM_SEND_DATA:		//发送请求
			{
				//效验数据
				tagSendData * pSendDataRequest = (tagSendData *)pData;
				assert(wDataSize >= (sizeof(tagSendData) - sizeof(pSendDataRequest->cbSendBuffer)));
				assert(wDataSize == (pSendDataRequest->wDataSize + sizeof(tagSendData) - sizeof(pSendDataRequest->cbSendBuffer)));

				//获取对象
				CTCPNetworkItem* pTCPNetworkItem = GetNetworkItem(pSendDataRequest->dwSocketID);
				if (pTCPNetworkItem == NULL) return false;

				//发送数据
				std::lock_guard<std::mutex> _lock(pTCPNetworkItem->GetMutex());
				pTCPNetworkItem->SendData(pSendDataRequest->wMainCmdID, pSendDataRequest->wSubCmdID, pSendDataRequest->cbSendBuffer, pSendDataRequest->wDataSize);
				return true;
			}
			case ASYNCHRONISM_SEND_BATCH:
			{
				//效验数据
				tagBatchSendRequest * pBatchSendRequest = (tagBatchSendRequest *)pData;
				assert(wDataSize >= (sizeof(tagBatchSendRequest) - sizeof(pBatchSendRequest->cbSendBuffer)));
				assert(wDataSize == (pBatchSendRequest->wDataSize + sizeof(tagBatchSendRequest) - sizeof(pBatchSendRequest->cbSendBuffer)));

				//群发数据
				CTCPNetworkItem * pTCPNetworkItem = nullptr;
				for (std::map<uint32, CTCPNetworkItem*>::iterator it = m_BatchNetItemStore.begin(); it != m_BatchNetItemStore.end(); ++it)
				{
					//获取对象
					pTCPNetworkItem = it->second;
				
					//发送数据
					std::lock_guard<std::mutex> _lock(pTCPNetworkItem->GetMutex());
					pTCPNetworkItem->SendData(pBatchSendRequest->wMainCmdID, pBatchSendRequest->wSubCmdID, pBatchSendRequest->cbSendBuffer, pBatchSendRequest->wDataSize);
				}
				return true;
			}
			case ASYNCHRONISM_CLOSE_SOCKET:
			{
				//效验数据
				assert(wDataSize == sizeof(tagCloseSocket));
				tagCloseSocket * pCloseSocket = (tagCloseSocket *)pData;

				//获取对象
				CTCPNetworkItem * pTCPNetworkItem = GetNetworkItem(pCloseSocket->dwSocketID);
				if (pTCPNetworkItem == nullptr) return false;

				//关闭连接
				std::lock_guard<std::mutex> _lock(pTCPNetworkItem->GetMutex());
				pTCPNetworkItem->CloseSocket();
			}
			case ASYNCHRONISM_SHUT_DOWN:
			{
				//效验数据
				assert(wDataSize == sizeof(tagShutDownSocket));
				tagShutDownSocket * pShutDownSocket = (tagShutDownSocket *)pData;

				//获取对象
				CTCPNetworkItem * pTCPNetworkItem = GetNetworkItem(pShutDownSocket->dwSocketID);
				if (pTCPNetworkItem == NULL) return false;

				//安全关闭
				std::lock_guard<std::mutex> _lock(pTCPNetworkItem->GetMutex());
				pTCPNetworkItem->DelayedCloseSocket();

				return true;
			}
			case ASYNCHRONISM_ALLOW_BATCH:
			{
				//效验数据
				assert(wDataSize == sizeof(tagAllowBatchSend));
				tagAllowBatchSend * pAllowBatchSend = (tagAllowBatchSend *)pData;

				//获取对象
				CTCPNetworkItem * pTCPNetworkItem = GetNetworkItem(pAllowBatchSend->dwSocketID);
				if (pTCPNetworkItem == nullptr) return false;

				//设置群发
				std::lock_guard<std::mutex> _lock(pTCPNetworkItem->GetMutex());
				pTCPNetworkItem->AllowBatchSend(pAllowBatchSend->cbAllowBatch);
				m_BatchNetItemStore[pTCPNetworkItem->GetIndex()] = pTCPNetworkItem;
			}
		}
		return false;
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

	bool CTCPNetworkEngine::SendData(uint32 dwSocketID, uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		//缓冲锁定
		std::lock_guard<std::mutex> _lock(m_mutex);
		tagSendData * pSendDataRequest = (tagSendData *)m_cbBuffer;

		//构造数据
		pSendDataRequest->wDataSize = wDataSize;
		pSendDataRequest->wSubCmdID = wSubCmdID;
		pSendDataRequest->wMainCmdID = wMainCmdID;
		pSendDataRequest->dwSocketID = dwSocketID;
		if (wDataSize > 0)
		{
			assert(pData != NULL);
			memcpy(pSendDataRequest->cbSendBuffer, pData, wDataSize);
		}

		//发送请求
		uint16 wSendSize = sizeof(tagSendData) - sizeof(pSendDataRequest->cbSendBuffer) + wDataSize;
		return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_SEND_DATA, m_cbBuffer, wSendSize);
	}

	bool CTCPNetworkEngine::SendDataBatch(uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		//效验数据
		assert((wDataSize + sizeof(TCP_Head)) <= SOCKET_TCP_PACKET);
		if ((wDataSize + sizeof(TCP_Head)) > SOCKET_TCP_PACKET) return false;

		//缓冲锁定
		std::lock_guard<std::mutex> _lock(m_mutex);
		tagBatchSendRequest * pBatchSendRequest = (tagBatchSendRequest *)m_cbBuffer;

		//构造数据
		pBatchSendRequest->wMainCmdID = wMainCmdID;
		pBatchSendRequest->wSubCmdID = wSubCmdID;
		pBatchSendRequest->wDataSize = wDataSize;

		if (wDataSize > 0)
		{
			assert(pData != nullptr);
			memcpy(pBatchSendRequest->cbSendBuffer, pData, wDataSize);
		}

		//发送请求
		uint16 wSendSize = sizeof(tagBatchSendRequest) - sizeof(pBatchSendRequest->cbSendBuffer) + wDataSize;
		return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_SEND_BATCH, m_cbBuffer, wSendSize);
	}

	bool CTCPNetworkEngine::CloseSocket(uint32 dwSocketID)
	{
		std::lock_guard<std::mutex> _lock(m_mutex);
		tagCloseSocket *pCloseSocket = (tagCloseSocket*)m_cbBuffer;
		pCloseSocket->dwSocketID = dwSocketID;
		return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_CLOSE_SOCKET, m_cbBuffer, sizeof(tagCloseSocket));
	}

	bool CTCPNetworkEngine::ShutDownSocket(uint32 dwSocketID)
	{
		std::lock_guard<std::mutex> _lock(m_mutex);
		tagShutDownSocket *pCloseSocket = (tagShutDownSocket*)m_cbBuffer;
		pCloseSocket->dwSocketID = dwSocketID;
		return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_SHUT_DOWN, m_cbBuffer, sizeof(tagShutDownSocket));
	}

	bool CTCPNetworkEngine::AllowBatchSend(uint32 dwSocketID, bool bAllowBatch)
	{
		std::lock_guard<std::mutex> _lock(m_mutex);
		tagAllowBatchSend *pAllowBatch = (tagAllowBatchSend*)m_cbBuffer;
		pAllowBatch->dwSocketID = dwSocketID;
		pAllowBatch->cbAllowBatch = bAllowBatch;
		return m_AsynchronismEngine.PostAsynchronismData(ASYNCHRONISM_ALLOW_BATCH, m_cbBuffer, sizeof(tagAllowBatchSend));
	}

	bool CTCPNetworkEngine::Start(Net::IOContext* ioContext)
	{
		//异步引擎
		IUnknownEx * pIUnknownEx = QUERY_ME_INTERFACE(IUnknownEx);
		if (m_AsynchronismEngine.SetAsynchronismSink(pIUnknownEx) == false)
		{
			assert(nullptr);
			return false;
		}

		//启动服务
		if (m_AsynchronismEngine.Start(ioContext) == false)
		{
			assert(nullptr);
			return false;
		}

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
#ifdef FORCE_CLOSE
		if (!m_NetworkFreeItem.empty())
		{
			pTCPNetworkItem = m_NetworkFreeItem.front();
			m_NetworkFreeItem.pop_front();
			pTCPNetworkItem->Attach(std::move(_socket));
		}
#endif
		//创建对象
		if (pTCPNetworkItem == nullptr)
		{
			pTCPNetworkItem = std::make_shared<CTCPNetworkItem>(m_curIndex, std::move(_socket), this);
			//pTCPNetworkItem = std::make_shared<CTCPNetworkItem>(m_curIndex++, std::move(_socket), this
			//		std::bind(&CTCPNetworkEngine::OnEventSocketBind, this, std::placeholders::_1),
			//		std::bind(&CTCPNetworkEngine::OnEventSocketShut, this, std::placeholders::_1),
			//		std::bind(&CTCPNetworkEngine::OnEventSocketRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
			++m_curIndex;
		}

		m_NetItemStore.insert(std::make_pair(pTCPNetworkItem->GetIndex(), pTCPNetworkItem.get()));
		return pTCPNetworkItem;
	}

	CTCPNetworkItem* CTCPNetworkEngine::GetNetworkItem(uint32 dwSocket)
	{
		auto k = m_NetItemStore.find(dwSocket);
		if (k != m_NetItemStore.end())
		{
			return m_NetItemStore[dwSocket];
		}
		return nullptr;
	}

	bool CTCPNetworkEngine::FreeNetworkItem(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem)
	{
		std::unordered_map<uint32, CTCPNetworkItem*>::iterator it = m_NetItemStore.find(pTCPNetworkItem->GetIndex());
		if (it == m_NetItemStore.end())
		{
			assert(nullptr);
			return false;
		}
		std::map<uint32, CTCPNetworkItem*>::iterator itBatch = m_BatchNetItemStore.find(pTCPNetworkItem->GetIndex());
		if (itBatch != m_BatchNetItemStore.end())
		{
			m_BatchNetItemStore.erase(itBatch);
		}
		m_NetItemStore.erase(it);
		
#ifdef FORCE_CLOSE
		m_NetworkFreeItem.push_back(pTCPNetworkItem);
#endif
		return true;
	}

	CTCPNetworkThread<CTCPNetworkItem>* CTCPNetworkEngine::CreateThreads()
	{
		return new CTCPNetworkThread<CTCPNetworkItem>[1];
	}

	DECLARE_CREATE_MODULE(TCPNetworkEngine);
}