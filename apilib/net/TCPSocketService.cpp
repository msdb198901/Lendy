#include "TCPSocketService.h"
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
#include <WinSock2.h>

#define EINPROGRESS WSAEINPROGRESS
#define SHUT_RDWR SD_BOTH
#endif

namespace Net
{
#define REQUEST_CONNECT				1									//请求连接
#define REQUEST_SEND_DATA			2									//请求发送
#define REQUEST_CLOSE_SOCKET		4									//请求关闭

	//连接错误
#define CONNECT_SUCCESS				0									//连接成功
#define CONNECT_FAILURE				1									//连接失败
#define CONNECT_EXCEPTION			2									//参数异常


	//关闭原因
#define SHUT_REASON_INSIDE			0									//内部原因
#define SHUT_REASON_NORMAL			1									//正常关闭
#define SHUT_REASON_REMOTE			2									//远程关闭
#define SHUT_REASON_TIME_OUT		3									//网络超时
#define SHUT_REASON_EXCEPTION		4									//异常关闭

#define IS_SS_RUN \
	bool bRun = m_TCPSocketServiceThread.IsStart(); \
	assert(bRun);	\
	if (!bRun) return false; 

#define IS_SS_STOP \
	bool bRun = m_TCPSocketServiceThread.IsStart(); \
	assert(!bRun);	\
	if (bRun) return false; 

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (char*)(address) - \
                                                  (ullong)(&((type *)0)->field)))
	//连接请求
	struct tagConnectRequest
	{
		uint16							wPort;								//连接端口
		uint64							dwServerIP;							//连接地址
	};

	//发送请求
	struct tagSendDataRequest
	{
		uint16							wMainCmdID;							//主命令码
		uint16							wSubCmdID;							//子命令码
		uint16							wDataSize;							//数据大小
		uint8							cbSendBuffer[SOCKET_TCP_PACKET];	//发送缓冲
	};

	CTCPSocketServiceThread::CTCPSocketServiceThread():
		m_ioContext(2),
		m_pStrand(new Net::Strand(m_ioContext)),
		m_pThread(nullptr),
		m_pThreadSelect(nullptr),
		m_wRecvSize(0)
	{
		memset(m_cbRecvBuf, 0, sizeof(m_cbRecvBuf));
	}

	CTCPSocketServiceThread::~CTCPSocketServiceThread()
	{
		Stop();
	}

	bool CTCPSocketServiceThread::Start()
	{
		if (m_pThread = new std::thread(&CTCPSocketServiceThread::Run, this), m_pThread == nullptr)
		{
			assert(nullptr);
			return false;
		}
		return true;
	}

	bool CTCPSocketServiceThread::Stop()
	{
		m_ioContext.stop();
		if (m_pThread)
		{
			m_pThread->join();
			PDELETE(m_pThread);
		}
		if (m_pThreadSelect)
		{
			m_pThreadSelect->join();
			PDELETE(m_pThreadSelect);
		}
		PDELETE(m_pStrand);
		return true;
	}

	bool CTCPSocketServiceThread::PostThreadRequest(uint16 wIdentifier, void * const pBuffer, uint16 wDataSize)
	{
		{
			std::lock_guard<std::mutex> _lock(m_mutex);
			m_dataQueue.InsertData(wIdentifier, pBuffer, wDataSize);
		}

		Net::post(m_ioContext, Net::bind_executor(*m_pStrand, [this, wIdentifier, pBuffer, wDataSize]() { 
			OnServiceRequest(wIdentifier, pBuffer, wDataSize);
		}));
		return true;
	}

	bool Net::CTCPSocketServiceThread::OnServiceRequest(uint16 wIdentifier, void * const pBuffer, uint16 wDataSize)
	{
		Util::tagDataHead DataHead;
		std::lock_guard<std::mutex> _lock(m_mutex);

		//提取数据
		uint8 cbBuffer[SOCKET_TCP_BUFFER] = {};
		if (m_dataQueue.DistillData(DataHead, cbBuffer, sizeof(cbBuffer)) == false) return 0;

		//数据处理
		switch (DataHead.wIdentifier)
		{
			case REQUEST_CONNECT:		//连接请求
			{
				//效验数据
				assert(DataHead.wDataSize == sizeof(tagConnectRequest));
				tagConnectRequest * pConnectRequest = (tagConnectRequest *)cbBuffer;

				//事件通知
				uint64 uConnect = PerformConnect(pConnectRequest->dwServerIP, pConnectRequest->wPort);
				int iErrorCode;
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
				iErrorCode = WSAGetLastError();
#else
				iErrorCode = errno;
#endif
				CTCPSocketService * pTCPSocketStatusService = CONTAINING_RECORD(this, CTCPSocketService, m_TCPSocketServiceThread);
				pTCPSocketStatusService->OnSocketLink(uConnect == CONNECT_SUCCESS ? 0 : iErrorCode);
				return true;
			}
			case REQUEST_SEND_DATA:
			{
				//效验数据
				tagSendDataRequest * pSendDataRequest = (tagSendDataRequest *)cbBuffer;
				assert(DataHead.wDataSize >= (sizeof(tagSendDataRequest) - sizeof(pSendDataRequest->cbSendBuffer)));
				assert(DataHead.wDataSize == (pSendDataRequest->wDataSize + sizeof(tagSendDataRequest) - sizeof(pSendDataRequest->cbSendBuffer)));

				//数据处理
				PerformSendData(pSendDataRequest->wMainCmdID, pSendDataRequest->wSubCmdID, pSendDataRequest->cbSendBuffer, pSendDataRequest->wDataSize);
				return true;
			}
			case REQUEST_CLOSE_SOCKET:
			{
				return true;
			}
		}
		return false;
	}

	uint64 CTCPSocketServiceThread::PerformConnect(uint64 dwServerIP, uint32 wPort)
	{
		try
		{
			if (!OnEventSocketCreate(AF_INET, SOCK_STREAM, IPPROTO_TCP))
			{
				throw CONNECT_EXCEPTION;
			}

			if (m_pThreadSelect = new std::thread(&CTCPSocketServiceThread::Loop, this), m_pThreadSelect == nullptr)
			{
				throw CONNECT_EXCEPTION;
			}

			struct sockaddr_in socketAddr;
			memset(&socketAddr, 0, sizeof(socketAddr));

			//设置变量
			socketAddr.sin_family = AF_INET;
			socketAddr.sin_port = htons(wPort);
			socketAddr.sin_addr.s_addr = dwServerIP;

			int nErrorCode = connect(m_hSocket, (const struct sockaddr *)&socketAddr, sizeof(socketAddr));
			if (nErrorCode == SOCKET_ERROR)
			{
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
				if (WSAGetLastError() != WSAEWOULDBLOCK) throw CONNECT_EXCEPTION;
#else
				throw CONNECT_EXCEPTION;
#endif
			}
			return CONNECT_SUCCESS;
		}
		catch (...)
		{

		}
		
		return CONNECT_EXCEPTION;
	}

	uint64 Net::CTCPSocketServiceThread::PerformSendData(uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		//效验状态
		if (m_hSocket == INVALID_SOCKET) return 0L;
		//if (m_TCPSocketStatus != SOCKET_STATUS_CONNECT) return 0L;

		//效验大小
		assert(wDataSize <= SOCKET_TCP_PACKET);
		if (wDataSize > SOCKET_TCP_PACKET) return 0L;

		//构造数据
		uint8 cbDataBuffer[SOCKET_TCP_BUFFER];
		TCP_Head * pHead = (TCP_Head *)cbDataBuffer;

		//设置变量
		pHead->CommandInfo.wSubCmdID = wSubCmdID;
		pHead->CommandInfo.wMainCmdID = wMainCmdID;

		//附加数据
		if (wDataSize > 0)
		{
			assert(pData != NULL);
			memcpy(pHead + 1, pData, wDataSize);
		}

		//加密数据
		uint16 wSendSize = EncryptBuffer(cbDataBuffer, sizeof(TCP_Head) + wDataSize, sizeof(cbDataBuffer));

		//发送数据
		return SendBuffer(cbDataBuffer, wSendSize);
	}

	bool Net::CTCPSocketServiceThread::OnEventSocketCreate(int af, int type, int protocol)
	{
		m_hSocket = ::socket(af, type, protocol);
		if (m_hSocket == INVALID_SOCKET) return false;

//		int iResult = 0;
//		int iShutdwonResult = 0;
//#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
//		unsigned long off;
//		iResult = ioctlsocket(m_hSocket, FIONBIO, &off);
//#else
//		int flags = fcntl(m_hSocket, F_GETFL);
//		iResult = fcntl(m_hSocket, F_SETFL, flags | O_NONBLOCK);
//#endif
//		if (iResult == SOCKET_ERROR)
//		{
//			if (m_hSocket != INVALID_SOCKET)
//			{
//				iShutdwonResult = shutdown(m_hSocket, SHUT_RDWR);
//			}
//			return false;
//		}
		return true;
	}

	bool Net::CTCPSocketServiceThread::OnSocketNotifyRead()
	{
		try
		{
			int iRetCode = recv(m_hSocket, (char*)m_cbRecvBuf + m_wRecvSize, sizeof(m_cbRecvBuf) - m_wRecvSize, 0);
			if (iRetCode == SOCKET_ERROR) throw "网络连接关闭，读取数据失败";

			m_wRecvSize += iRetCode;

			uint16 wPacketSize = 0;
			uint8 cbDataBuffer[SOCKET_TCP_PACKET + sizeof(TCP_Head)];

			TCP_Head *pHead = (TCP_Head*)m_cbRecvBuf;

			while (m_wRecvSize >= sizeof(TCP_Head))
			{
				wPacketSize = pHead->TCPInfo.wPacketSize;
				assert(pHead->TCPInfo.cbDataKind == DK_MAPPED);
				assert(wPacketSize <= (SOCKET_TCP_PACKET + sizeof(TCP_Head)));

				if (pHead->TCPInfo.cbDataKind != DK_MAPPED) throw "数据包版本错误";
				if (wPacketSize > (SOCKET_TCP_PACKET + sizeof(TCP_Head))) throw "数据包太大";
				if (m_wRecvSize < wPacketSize) return false;

				memcpy(cbDataBuffer, m_cbRecvBuf, wPacketSize);
				m_wRecvSize -= wPacketSize;
				memmove(m_cbRecvBuf, m_cbRecvBuf + wPacketSize, m_wRecvSize);

				//解密数据
				uint16 wRealySize = CrevasseBuffer(cbDataBuffer, wPacketSize);
				assert(wRealySize >= sizeof(TCP_Head));

				//解释数据
				uint16 wDataSize = wRealySize - sizeof(TCP_Head);
				void * pDataBuffer = cbDataBuffer + sizeof(TCP_Head);
				TCP_Command Command = ((TCP_Head *)cbDataBuffer)->CommandInfo;

				//内核数据
				if (Command.wMainCmdID == MDM_KN_COMMAND)
				{
					switch (Command.wSubCmdID)
					{
						case SUB_KN_DETECT_SOCKET:		//网络检测
						{
							//回应数据
							PerformSendData(MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);

							break;
						}
					}
				}
				else
				{
					//处理数据
					CTCPSocketService * pTCPSocketStatusService = CONTAINING_RECORD(this, CTCPSocketService, m_TCPSocketServiceThread);
					if (pTCPSocketStatusService->OnSocketRead(Command, pDataBuffer, wDataSize) == false) throw "网络数据包处理失败";
				}
			}

		}
		catch (...)
		{
			//关闭连接
			PerformCloseSocket(SHUT_REASON_NORMAL);
		}
		return true;
	}

	bool Net::CTCPSocketServiceThread::OnSocketNotifyWrite()
	{
		return false;
	}

	void Net::CTCPSocketServiceThread::PerformCloseSocket(uint8 cbShutReason)
	{
		//关闭判断
		if (m_hSocket != INVALID_SOCKET)
		{
			//关闭连接
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
			closesocket(m_hSocket);
#else
			close(m_hSocket);
#endif
			m_hSocket = INVALID_SOCKET;

			//关闭通知
			if (cbShutReason != SHUT_REASON_INSIDE)
			{
				CTCPSocketService * pTCPSocketStatusService = CONTAINING_RECORD(this, CTCPSocketService, m_TCPSocketServiceThread);
				pTCPSocketStatusService->OnSocketShut(cbShutReason);
			}
		}
		return;
	}

	uint64 Net::CTCPSocketServiceThread::SendBuffer(void * pBuffer, uint16 wSendSize)
	{
		//变量定义
		uint16 wTotalCount = 0;

		//发送数据
		while (wTotalCount < wSendSize)
		{
			//发生数据
			int nSendCount = send(m_hSocket, (char *)pBuffer + wTotalCount, wSendSize - wTotalCount, 0);

			//错误判断
			if (nSendCount == SOCKET_ERROR)
			{
				////缓冲判断
				//if (WSAGetLastError() == WSAEWOULDBLOCK)
				//{
				//	AmortizeBuffer((LPBYTE)pBuffer + wTotalCount, wSendSize - wTotalCount);
				//	return wSendSize;
				//}
				////关闭连接
				//PerformCloseSocket(SHUT_REASON_EXCEPTION);

				return 0L;
			}
			else
			{
				//设置变量
				wTotalCount += nSendCount;
			}
		}

		////缓冲数据
		//if (wTotalCount > wSendSize)
		//{
		//	AmortizeBuffer((LPBYTE)pBuffer + wTotalCount, wSendSize - wTotalCount);
		//}

		return wSendSize;
	}

	uint16 Net::CTCPSocketServiceThread::CrevasseBuffer(uint8 cbDataBuffer[], uint16 wDataSize)
	{
		//效验参数
		assert(wDataSize >= sizeof(TCP_Head));
		assert(((TCP_Head *)cbDataBuffer)->TCPInfo.wPacketSize == wDataSize);

		//效验码与字节映射
		TCP_Head * pHead = (TCP_Head *)cbDataBuffer;
		for (int i = sizeof(TCP_Info); i < wDataSize; i++)
		{
			cbDataBuffer[i] = g_RecvByteMap[cbDataBuffer[i]];
		}

		return wDataSize;
	}

	uint16 Net::CTCPSocketServiceThread::EncryptBuffer(uint8 cbDataBuffer[], uint16 wDataSize, uint16 wBufferSize)
	{
		int i = 0;
		//效验参数
		assert(wDataSize >= sizeof(TCP_Head));
		assert(wBufferSize >= (wDataSize + 2 * sizeof(uint32)));
		assert(wDataSize <= (sizeof(TCP_Head) + SOCKET_TCP_BUFFER));

		//填写信息头
		TCP_Head * pHead = (TCP_Head *)cbDataBuffer;
		pHead->TCPInfo.wPacketSize = wDataSize;
		pHead->TCPInfo.cbDataKind = DK_MAPPED;


		uint8 checkCode = 0;

		for (uint16 i = sizeof(TCP_Info); i < wDataSize; i++)
		{
			checkCode += cbDataBuffer[i];
			cbDataBuffer[i] = g_SendByteMap[cbDataBuffer[i]];
		}
		pHead->TCPInfo.cbCheckCode = ~checkCode + 1;

		return wDataSize;
	}

	void CTCPSocketServiceThread::Run()
	{
		//事件通知
		asio::io_context::work work(m_ioContext);
		m_ioContext.run();
	}

	void Net::CTCPSocketServiceThread::Loop()
	{
		try
		{
			fd_set readset, o_readset;
			fd_set writeset, o_writeset;
			fd_set exceptset, o_exceptset;
			
			m_tTimeOut.tv_sec = 0;
			m_tTimeOut.tv_usec = 0;

			FD_ZERO(&readset);
			FD_ZERO(&writeset);
			FD_ZERO(&exceptset);
			FD_SET(m_hSocket, &readset);
			FD_SET(m_hSocket, &writeset);
			FD_SET(m_hSocket, &exceptset);

			o_readset = readset;
			o_writeset = writeset;
			o_exceptset = exceptset;

			while (true)
			{
				if (m_hSocket == INVALID_SOCKET) break;

#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
				Sleep(100);
#else
				usleep(100 * 1000);
#endif
				readset = o_readset;
				writeset = o_writeset;
				exceptset = o_exceptset;

				int result = select(m_hSocket + 1, &readset, &writeset, &exceptset, &m_tTimeOut);
				if (result == SOCKET_ERROR)
				{
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
					int iErrorCode = WSAGetLastError();
#else
					int iErrorCode = errno;
#endif
					(void)iErrorCode;
					assert(nullptr);
					break;
				}

				if (result <= 0) continue;

				//可读
				if (FD_ISSET(m_hSocket, &readset))
				{
					OnSocketNotifyRead();
				}
				else if (FD_ISSET(m_hSocket, &writeset))
				{
					OnSocketNotifyWrite();
				}
				else if (FD_ISSET(m_hSocket, &exceptset))
				{
					//printf("11111111\n");
					int error = 0;
					socklen_t len = 128;
					char sz[128] = {};
					if (getsockopt(m_hSocket, SOL_SOCKET, SO_ERROR, sz, &len) < 0)
					{
						break;
					}
					int i = 0;
					++i;
				}
			}
		}
		catch (...)
		{

		}
	}

	CTCPSocketService::CTCPSocketService() :
		m_wServiceID(0),
		m_pITCPSocketEvent(nullptr)
	{
		memset(m_cbBuffer, 0, sizeof(m_cbBuffer));
	}

	CTCPSocketService::~CTCPSocketService()
	{
		Stop();
	}

	void * CTCPSocketService::QueryInterface(GGUID uuid)
	{
		QUERY_INTERFACE(IServiceModule, uuid);
		QUERY_INTERFACE(ITCPSocketService, uuid);
		QUERY_INTERFACE_IUNKNOWNEX(ITCPSocketService, uuid);
		return nullptr;
	}

	bool CTCPSocketService::Start(Net::IOContext * ioContext)
	{
		IS_SS_STOP
		if (!m_TCPSocketServiceThread.Start())
		{
			return false;
		}
		return true;
	}

	bool CTCPSocketService::Stop()
	{
		IS_SS_RUN
		m_TCPSocketServiceThread.Stop();
		return true;
	}

	bool CTCPSocketService::SetServiceID(uint16 wServiceID)
	{
		IS_SS_STOP
		m_wServiceID = wServiceID;
		return true;
	}

	bool CTCPSocketService::SetTCPSocketEvent(IUnknownEx * pIUnknownEx)
	{
		IS_SS_STOP
		m_pITCPSocketEvent = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, ITCPSocketEvent);

		//错误判断
		if (m_pITCPSocketEvent == nullptr)
		{
			assert(nullptr);
			return false;
		}
		return true;
	}

	bool CTCPSocketService::CloseSocket()
	{
		IS_SS_RUN
		std::lock_guard<std::mutex> _lock(m_mutex);
		return m_TCPSocketServiceThread.PostThreadRequest(REQUEST_CLOSE_SOCKET, nullptr, 0);
	}

	bool CTCPSocketService::Connect(uint64 dwServerIP, uint16 wPort)
	{
		IS_SS_RUN
		std::lock_guard<std::mutex> _lock(m_mutex);

		tagConnectRequest *pConnectRequest = (tagConnectRequest *)m_cbBuffer;
		pConnectRequest->wPort = wPort;
		pConnectRequest->dwServerIP = htonl(dwServerIP);
		return m_TCPSocketServiceThread.PostThreadRequest(REQUEST_CONNECT, &m_cbBuffer, sizeof(tagConnectRequest));
	}

	bool CTCPSocketService::Connect(std::string strServerIP, uint16 wPort)
	{
		IS_SS_RUN
		std::lock_guard<std::mutex> _lock(m_mutex);

		tagConnectRequest *pConnectRequest = (tagConnectRequest *)m_cbBuffer;
		pConnectRequest->wPort = wPort;
		pConnectRequest->dwServerIP = inet_addr(strServerIP.c_str());
		return m_TCPSocketServiceThread.PostThreadRequest(REQUEST_CONNECT, &m_cbBuffer, sizeof(tagConnectRequest));
	}

	bool CTCPSocketService::SendData(uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		//状态效验
		IS_SS_RUN
		std::lock_guard<std::mutex> _lock(m_mutex);

		//构造数据
		tagSendDataRequest SendRequest;
		memset(&SendRequest, 0, sizeof(SendRequest));

		//设置变量
		SendRequest.wDataSize = wDataSize;
		SendRequest.wSubCmdID = wSubCmdID;
		SendRequest.wMainCmdID = wMainCmdID;

		//附加数据
		if (wDataSize > 0)
		{
			assert(pData != NULL);
			memcpy(SendRequest.cbSendBuffer, pData, wDataSize);
		}

		//投递请求
		uint16 wSendSize = sizeof(SendRequest) - sizeof(SendRequest.cbSendBuffer) + wDataSize;
		return m_TCPSocketServiceThread.PostThreadRequest(REQUEST_SEND_DATA, &SendRequest, wSendSize);
	}

	bool CTCPSocketService::OnSocketLink(int nErrorCode)
	{
		//投递事件
		assert(m_pITCPSocketEvent != nullptr);
		return m_pITCPSocketEvent->OnEventTCPSocketLink(m_wServiceID, nErrorCode);
	}

	bool Net::CTCPSocketService::OnSocketShut(uint8 cbShutReason)
	{
		//投递事件
		assert(m_pITCPSocketEvent != nullptr);
		return m_pITCPSocketEvent->OnEventTCPSocketShut(m_wServiceID, cbShutReason);
	}

	//读取消息
	bool CTCPSocketService::OnSocketRead(TCP_Command Command, void * pData, uint16 wDataSize)
	{
		//投递事件
		assert(m_pITCPSocketEvent != NULL);
		return m_pITCPSocketEvent->OnEventTCPSocketRead(m_wServiceID, Command, pData, wDataSize);
	}

	//组件创建函数
	DECLARE_CREATE_MODULE(TCPSocketService);
}