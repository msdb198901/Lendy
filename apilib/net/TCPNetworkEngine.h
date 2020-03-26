#ifndef TCP_NETWORK_ENGINE_H
#define TCP_NETWORK_ENGINE_H

#include "netdef/AsyncAcceptor.h"
#include "../KernelEngineHead.h"
#include "AsynchronismEngine.h"
#include "TCPNetworkThread.h"
#include "TCPNetworkItem.h"


#include <unordered_map>

namespace Net
{
	class CTCPNetworkEngine : public ITCPNetworkEngine, public ITCPNetworkItemSink, public IAsynchronismEngineSink
	{
		//函数定义
	public:
		//构造函数
		CTCPNetworkEngine();

		//析构函数
		virtual ~CTCPNetworkEngine();

		//基础接口
	public:
		//释放对象
		virtual void Release();
		//接口查询
		virtual void * QueryInterface(GGUID uuid);

	public:
		//设置调度
		virtual bool SetTCPNetworkEngineEvent(IUnknownEx * pIUnknownEx);
		//设置参数
		virtual bool SetServiceParameter(std::string strBindIP, uint16 port, uint16 threadCount);

		//异步接口
	public:
		//启动事件
		virtual bool OnAsynchronismEngineStart() { return true; }
		//停止事件
		virtual bool OnAsynchronismEngineConclude() { return true; }
		//异步数据
		virtual bool OnAsynchronismEngineData(uint16 wIdentifier, VOID * pData, uint16 wDataSize);

	public:
		//绑定事件
		virtual bool OnEventSocketBind(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem);
		//关闭事件
		virtual bool OnEventSocketShut(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem);
		//读取事件
		virtual bool OnEventSocketRead(TCP_Command Command, void * pData, uint16 wDataSize, std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem);

	public:
		//发送函数
		virtual bool SendData(uint64 dwSocketID, uint16 wMainCmdID, uint16 wSubCmdID, void * pData = nullptr, uint16 wDataSize = 0);

		//控制接口
	public:
		//关闭连接
		virtual bool CloseSocket(uint64 dwSocketID);

		//服务接口
	public:
		//启动服务
		virtual bool Start(Net::IOContext* ioContext);

		//停止服务
		virtual bool Stop();

		void OnSocketOpen(tcp::socket &&_socket, uint32 threadIndex);

		int GetNetworkThreadCount() const;

		uint32 SelectThreadWithMinConnections() const;

		std::pair<tcp::socket*, uint32> GetAcceptSocket();

	public:
		void Wait();

		//对象管理
	protected:
		//激活空闲对象
		std::shared_ptr<CTCPNetworkItem> ActiveNetworkItem(tcp::socket && _socket);
		//获取对象
		CTCPNetworkItem* GetNetworkItem(uint16 wIndex);
		//释放连接对象
		bool FreeNetworkItem(std::shared_ptr<CTCPNetworkItem> pTCPNetworkItem);

	protected:
		virtual CTCPNetworkThread<CTCPNetworkItem>* CreateThreads();

	private:
		uint16											m_curIndex;
		std::string										m_strBindIP;
		uint16											m_port;
		AsyncAcceptor*									m_acceptor;

		uint16											m_threadCount;
		CTCPNetworkThread<CTCPNetworkItem>*				m_pThreads;
		std::deque<std::shared_ptr<CTCPNetworkItem>>	m_NetworkFreeItem;

		ITCPNetworkEngineEvent*							m_pITCPNetworkEngineEvent;			//事件接口

	private:
		CAsynchronismEngine								m_AsynchronismEngine;				//异步对象
		std::unordered_map<uint64, CTCPNetworkItem*>	m_NetItemStore;

		//辅助变量
	protected:
		std::mutex						m_mutex;
		BYTE							m_cbBuffer[SOCKET_TCP_BUFFER];		//临时对象
	};
}

#endif