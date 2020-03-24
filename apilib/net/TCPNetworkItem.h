#ifndef TCP_NETWORK_ITEM_H
#define TCP_NETWORK_ITEM_H

#include "Define.h"
#include "Packet.h"
#include "MessageBuffer.h"
#include <atomic>
#include <queue>
#include <memory>
#include <functional>
#include <asio/ip/tcp.hpp>

#define READ_BLOCK_SIZE 4096
#ifdef BOOST_ASIO_HAS_IOCP
#define LENDY_SOCKET_USE_IOCP
#endif

namespace Net
{
	using asio::ip::tcp;
	using Util::MessageBuffer;

	class CTCPNetworkItem;

	//连接对象回调接口
	struct ITCPNetworkItemSink
	{
		//绑定事件
		virtual bool OnEventSocketBind(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem) = 0;
		//关闭事件
		virtual bool OnEventSocketShut(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem) = 0;
		//读取事件
		virtual bool OnEventSocketRead(TCP_Command Command, void * pData, uint16 wDataSize, std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem) = 0;
	};

	class CTCPNetworkItem : public std::enable_shared_from_this<CTCPNetworkItem>
	{
	public:
		explicit CTCPNetworkItem(uint16 index, tcp::socket&& socket, ITCPNetworkItemSink*);
		//explicit CTCPNetworkItem(uint16 index, tcp::socket&& socket, ITCPNetworkItemSink*,
		//	std::function<bool(CTCPNetworkItem *)>,
		//	std::function<bool(CTCPNetworkItem *)>,
		//	std::function<bool(TCP_Command, void*, uint16, CTCPNetworkItem*)>);

		virtual ~CTCPNetworkItem();

		virtual void Start();

		virtual bool Update();

		void Attach(tcp::socket&& socket);

		asio::ip::address GetRemoteIPAddress() const;

		uint16 GetRemotePort() const;

		void AsyncRead();

		bool SendData(uint16 wMainCmdID, uint16 wSubCmdID, void *pData, uint16 wDataSize);

		void QueuePacket(MessageBuffer&& buffer);

		bool IsOpen() const;

		void CloseSocket();

		void DelayedCloseSocket(); 

		uint16 GetIndex() { return m_index; }
		uint64 GetClientIP() { return m_remoteAddress.to_v4().to_uint();}

		MessageBuffer& GetReadBuffer();

		//回调函数
	public:
		void SocketBindCallBack();

		void SocketShutCallBack();

	protected:
		virtual void OnClose();

		virtual void ReadHandler();

		bool AsyncProcessQueue();

		void SetNoDelay(bool enable);

		//加密函数
	private:
		//加密数据
		uint16 EncryptBuffer(uint8 pcbDataBuffer[], uint16 wDataSize, uint16 wBufferSize);
		//解密数据
		uint16 CrevasseBuffer(uint8 pcbDataBuffer[], uint16 wDataSize);

	private:
		void ReadHandlerInternal(asio::error_code error, size_t transferredBytes);

#ifdef LENDY_SOCKET_USE_IOCP
		void WriteHandler(asio::error_code error, std::size_t transferedBytes);
#endif
		void WriteHandlerWrapper(asio::error_code /*error*/, std::size_t /*transferedBytes*/);

		bool HandleQueue();

	private:
		uint16						m_index;
		tcp::socket					m_socket;
		asio::ip::address			m_remoteAddress;
		uint16						m_remotePort;

		MessageBuffer				m_readBuffer;
		std::queue<MessageBuffer>	m_writeQueue;

		std::atomic<bool>			m_closed;
		std::atomic<bool>			m_closing;
		bool						m_bWritingAsync;

		ITCPNetworkItemSink *		m_pITCPNetworkItemSink;				//回调接口

	//private:
	//	std::function<bool(CTCPNetworkItem *)> m_SocketBindCallBack;
	//	std::function<bool(CTCPNetworkItem *)> m_SocketShutCallBack;
	//	std::function<bool(TCP_Command, void*, uint16, CTCPNetworkItem*)> m_SocketReadCallBack;
	};
}

#endif