#include "TCPNetworkItem.h"

namespace Net
{
	CTCPNetworkItem::CTCPNetworkItem(uint16 index, tcp::socket && socket, 
		ITCPNetworkItemSink * pITCPNetworkItemSink/*,
		std::function<bool(CTCPNetworkItem *)> socketBindCallBack,
		std::function<bool(CTCPNetworkItem *)> socketShutCallBack,
		std::function<bool(TCP_Command, void*, uint16, CTCPNetworkItem*)> socketReadCallBack*/) :
		m_index(index),
		m_bAllowBatch(false),
		m_socket(std::move(socket)),
		m_pITCPNetworkItemSink(pITCPNetworkItemSink),
		m_remoteAddress(m_socket.remote_endpoint().address()),
		m_remotePort(m_socket.remote_endpoint().port()),
		m_readBuffer(),
		m_closed(false),
		m_closing(false),
		m_bWritingAsync(false)/*,
		m_SocketBindCallBack(socketBindCallBack),
		m_SocketShutCallBack(socketShutCallBack),
		m_SocketReadCallBack(socketReadCallBack)*/
	{
		m_readBuffer.Resize(READ_BLOCK_SIZE);
	}

	CTCPNetworkItem::~CTCPNetworkItem()
	{
#ifndef LENDY_SOCKET_FORCE_CLOSE
		m_closed = true;
		asio::error_code ec;
		m_socket.close(ec);
#endif
	}

	void CTCPNetworkItem::Start()
	{
		AsyncRead();
	}

	bool CTCPNetworkItem::Update()
	{
		if (m_closed)
		{
			return false;
		}

#ifndef LENDY_SOCKET_USE_IOCP
		if (m_bWritingAsync || (m_writeQueue.empty() && !m_closing))
			return true;
		for (; HandleQueue();)
			;
#endif
		return true;
	}

	void CTCPNetworkItem::Attach(tcp::socket && socket)
	{
		m_bAllowBatch = false;
		m_socket = std::move(socket);
		m_remoteAddress = m_socket.remote_endpoint().address();
		m_remotePort = m_socket.remote_endpoint().port();
		m_readBuffer.Reset();
		m_closed = false;
		m_closing = false;
		m_bWritingAsync = false;
	}

	asio::ip::address CTCPNetworkItem::GetRemoteIPAddress() const
	{
		return m_remoteAddress;
	}

	uint16 CTCPNetworkItem::GetRemotePort() const
	{
		return m_remotePort;
	}

	void CTCPNetworkItem::AsyncRead()
	{
		if (!IsOpen())
		{
			return;
		}

		m_readBuffer.Normalize();
		m_readBuffer.EnsureFreeSpace();
		m_socket.async_read_some(asio::buffer(m_readBuffer.GetWritePointer(), m_readBuffer.GetRemainingSpace()),
			std::bind(&CTCPNetworkItem::ReadHandlerInternal, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	bool CTCPNetworkItem::SendData(uint16 wMainCmdID, uint16 wSubCmdID, void * pData, uint16 wDataSize)
	{
		//变量定义
		uint16 wPacketSize = sizeof(TCP_Head) + wDataSize;
		MessageBuffer buff(wPacketSize);
		TCP_Head * pHead = (TCP_Head *)buff.GetWritePointer();

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
		uint16 wEncryptLen = EncryptBuffer(buff.GetWritePointer(), wPacketSize, buff.GetBufferSize());
		buff.WriteCompleted(wEncryptLen);
		QueuePacket(std::move(buff));
		return true;
	}

	void CTCPNetworkItem::QueuePacket(MessageBuffer && buffer)
	{
		m_writeQueue.push(std::move(buffer));

#ifdef LENDY_SOCKET_USE_IOCP
		AsyncProcessQueue();
#endif
	}

	bool CTCPNetworkItem::IsOpen() const
	{
		return !m_closed && !m_closing;
	}

	void CTCPNetworkItem::CloseSocket()
	{
		if (m_closed.exchange(true))
		{
			return;
		}
			
#ifndef LENDY_SOCKET_FORCE_CLOSE
		asio::error_code shutdownError;
		m_socket.shutdown(asio::socket_base::shutdown_send, shutdownError);
		if (shutdownError)
		{
			assert(nullptr);
		}
#else
		asio::error_code cancelError;
		m_socket.cancel(cancelError);
		if (cancelError)
		{
			assert(nullptr);
		}
		asio::error_code err;
		m_socket.close(err);
		if (err)
		{
			assert(nullptr);
		}
#endif
		OnClose();
	}

	void CTCPNetworkItem::DelayedCloseSocket()
	{
		m_closing = true;
	}

	bool CTCPNetworkItem::AllowBatchSend(bool cbAllowBatch)
	{
		if (!IsOpen())
		{
			return false;
		}

		m_bAllowBatch = cbAllowBatch;
		return true;
	}

	MessageBuffer & CTCPNetworkItem::GetReadBuffer()
	{
		return m_readBuffer;
	}

	void CTCPNetworkItem::SocketBindCallBack()
	{
		m_pITCPNetworkItemSink->OnEventSocketBind(shared_from_this());
	}

	void CTCPNetworkItem::SocketShutCallBack()
	{
		m_pITCPNetworkItemSink->OnEventSocketShut(shared_from_this());
	}

	void CTCPNetworkItem::OnClose()
	{

	}

	void CTCPNetworkItem::ReadHandler()
	{
		uint8 cbBuffer[SOCKET_TCP_BUFFER];
		MessageBuffer& packet = GetReadBuffer();
		TCP_Head * pHead = nullptr;

		try
		{
			while (packet.GetActiveSize() >= sizeof(TCP_Head))
			{
				pHead = (TCP_Head *)packet.GetReadPointer();

				//效验数据
				uint16 wPacketSize = pHead->TCPInfo.wPacketSize;

				//数据判断
				if (wPacketSize > SOCKET_TCP_BUFFER)
				{
					throw "数据包长度太长";
				}
				if (wPacketSize < sizeof(TCP_Head))
				{
					throw "数据包长度太短";
				}
				if (pHead->TCPInfo.cbDataKind != DK_MAPPED && pHead->TCPInfo.cbDataKind != 0x05)
				{
					throw "数据包版本不匹配";
				}

				//完成判断
				if (packet.GetActiveSize() < wPacketSize)
				{
					break;
				}

				//提取数据
				memcpy(cbBuffer, packet.GetReadPointer(), wPacketSize);
				uint16 wRealySize = CrevasseBuffer(cbBuffer, wPacketSize);

				//解释数据
				void* pData = cbBuffer + sizeof(TCP_Head);
				uint16 wDataSize = wRealySize - sizeof(TCP_Head);
				TCP_Command Command = ((TCP_Head *)cbBuffer)->CommandInfo;

				//消息处理
				if (Command.wMainCmdID != MDM_KN_COMMAND)
				{
					m_pITCPNetworkItemSink->OnEventSocketRead(Command, pData, wDataSize, shared_from_this());
				}

				packet.ReadCompleted(wPacketSize);
			}
		}
		catch (...)
		{
			CloseSocket();
			return ;
		}

		AsyncRead();
	}

	bool CTCPNetworkItem::AsyncProcessQueue()
	{
		if (m_bWritingAsync)
		{
			return false;
		}

		m_bWritingAsync = true;

#ifdef LENDY_SOCKET_USE_IOCP

#else
		m_socket.async_write_some(asio::null_buffers(), std::bind(&CTCPNetworkItem::WriteHandlerWrapper,
			this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
#endif

		return false;
	}

	void CTCPNetworkItem::SetNoDelay(bool enable)
	{
		asio::error_code err;
		m_socket.set_option(tcp::no_delay(enable), err);
		if (err)
		{
			assert(nullptr);
		}
	}

	uint16 CTCPNetworkItem::EncryptBuffer(uint8 pcbDataBuffer[], uint16 wDataSize, uint16 wBufferSize)
	{
		//效验参数
		assert(wDataSize >= sizeof(TCP_Head));
		assert(wDataSize <= (sizeof(TCP_Head) + SOCKET_TCP_BUFFER));
		(void)wBufferSize;
		//assert(wBufferSize >= (wDataSize + 2 * sizeof(uint32)));

		//填写信息头
		TCP_Head * pHead = (TCP_Head *)pcbDataBuffer;
		pHead->TCPInfo.wPacketSize = wDataSize;
		pHead->TCPInfo.cbDataKind = DK_MAPPED;

		uint8 checkCode = 0;
		for (uint16 i = sizeof(TCP_Info); i < wDataSize; i++)
		{
			checkCode += pcbDataBuffer[i];
			pcbDataBuffer[i] = g_SendByteMap[pcbDataBuffer[i]];
		}
		pHead->TCPInfo.cbCheckCode = ~checkCode + 1;

		return wDataSize;
	}

	uint16 CTCPNetworkItem::CrevasseBuffer(uint8 pcbDataBuffer[], uint16 wDataSize)
	{
		//效验参数
		assert(wDataSize >= sizeof(TCP_Head));
		assert(((TCP_Head *)pcbDataBuffer)->TCPInfo.wPacketSize == wDataSize);

		//效验码与字节映射
		TCP_Head * pHead = (TCP_Head *)pcbDataBuffer;
		for (uint16 i = sizeof(TCP_Info); i < wDataSize; i++)
		{
			pcbDataBuffer[i] = g_RecvByteMap[pcbDataBuffer[i]];
		}

		return wDataSize;
	}

	void CTCPNetworkItem::ReadHandlerInternal(asio::error_code error, size_t transferredBytes)
	{
		if (error)
		{
			CloseSocket();
			return;
		}

		m_readBuffer.WriteCompleted(transferredBytes);
		ReadHandler();
	}

#ifdef LENDY_SOCKET_USE_IOCP
	void CTCPNetworkItem::WriteHandler(asio::error_code error, std::size_t transferedBytes)
	{
		if (!error)
		{
			m_bWritingAsync = false;
			m_writeQueue.front().ReadCompleted(transferedBytes);
			if (!m_writeQueue.front().GetActiveSize())
			{
				m_writeQueue.pop();
			}

			if (!m_writeQueue.empty())
			{
				AsyncProcessQueue();
			}
			else if (m_closing)
			{
				CloseSocket();
			}
		}
		else
		{
			CloseSocket();
		}
	}
	
#endif

	void CTCPNetworkItem::WriteHandlerWrapper(asio::error_code, std::size_t)
	{
		m_bWritingAsync = false;
		HandleQueue();
	}

	bool CTCPNetworkItem::HandleQueue()
	{
		if (m_writeQueue.empty())
		{
			return false;
		}

		MessageBuffer &queueMessage = m_writeQueue.front();

		std::size_t bytesToSend = queueMessage.GetActiveSize();

		asio::error_code error;
		std::size_t bytesSend = m_socket.write_some(asio::buffer(queueMessage.GetReadPointer(), bytesToSend), error);

		if (error)
		{
			if (error == asio::error::would_block || error == asio::error::try_again)
			{
				return AsyncProcessQueue();
			}

			m_writeQueue.pop();

			if (m_closing && m_writeQueue.empty())
			{
				CloseSocket();
			}

			return false;
		}
		else if (bytesSend == 0)
		{
			m_writeQueue.pop();

			if (m_closing && m_writeQueue.empty())
			{
				CloseSocket();
			}

			return false;
		}
		else if (bytesSend < bytesToSend)
		{
			queueMessage.ReadCompleted(bytesSend);
			return AsyncProcessQueue();
		}

		m_writeQueue.pop();

		if (m_closing && m_writeQueue.empty())
		{
			CloseSocket();
		}

		return !m_writeQueue.empty();
	}


}