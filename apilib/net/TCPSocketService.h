#ifndef TCP_SOCKET_SERVICE_H
#define TCP_SOCKET_SERVICE_H

#include "../KernelEngineHead.h"
#include "Strand.h"
#include <thread>
#include <mutex>

namespace Net
{
	class CTCPSocketServiceThread
	{
		//函数定义
	public:
		//构造函数
		CTCPSocketServiceThread();
		//析构函数
		virtual ~CTCPSocketServiceThread();

		//服务接口
	public:
		//启动服务
		virtual bool Start();
		//停止服务
		virtual bool Stop();

		bool IsStart() { return m_pThread != nullptr; }

		//请求函数
	public:
		//投递请求
		bool PostThreadRequest(uint16 wIdentifier, void * const pBuffer, uint16 wDataSize);

		//处理函数
	private:
		//请求消息
		bool OnServiceRequest(uint16 wIdentifier, void * const pBuffer, uint16 wDataSize);

		//控制函数
	private:
		//连接服务器
		uint64 PerformConnect(uint64 dwServerIP, uint32 wPort);

		//发送函数
		uint64 PerformSendData(uint16 wMainCmdID, uint16 wSubCmdID, void * pData = nullptr, uint16 wDataSize = 0);

		//创建socket
		bool OnEventSocketCreate(int af, int type, int protocol);

		//辅助函数
	private:
		//发送数据
		uint64 SendBuffer(void * pBuffer, uint16 wSendSize);
		//解密数据
		uint16 CrevasseBuffer(uint8 cbDataBuffer[], uint16 wDataSize);
		//加密数据
		uint16 EncryptBuffer(uint8 cbDataBuffer[], uint16 wDataSize, uint16 wBufferSize);

		//内部运行
	protected:
		void Run();

		void Loop();

		//内核变量
	protected:
		SOCKET								m_hSocket;							//连接句柄
		SOCKET								m_hMinSocket;						//连接句柄
		SOCKET								m_hMaxSocket;						//连接句柄
		timeval								m_tTimeOut;

	protected:
		Net::IOContext 						m_ioContext;
		Net::Strand*						m_pStrand;
		std::thread*						m_pThread;
		std::thread*						m_pThreadSelect;
	};

	//网络服务
	class CTCPSocketService : public ITCPSocketService
	{
		//友元说明
		friend class CTCPSocketServiceThread;

	protected:
		uint16							m_wServiceID;						//服务标识
		ITCPSocketEvent*				m_pITCPSocketEvent;					//事件接口

		//辅助变量
	protected:
		std::mutex						m_mutex;
		uint8							m_cbBuffer[SOCKET_TCP_BUFFER];		//临时对象

		//组件变量
	protected:
		CTCPSocketServiceThread			m_TCPSocketServiceThread;			//网络线程

		//函数定义
	public:
		//构造函数
		CTCPSocketService();
		//析构函数
		virtual ~CTCPSocketService();

		//基础接口
	public:
		//释放对象
		virtual void Release() { delete this; }
		//接口查询
		virtual void *QueryInterface(GGUID uuid);

		//服务接口
	public:
		//启动服务
		virtual bool Start(Net::IOContext* ioContext);

		//停止服务
		virtual bool Stop();

		//配置接口
	public:
		//配置函数
		virtual bool SetServiceID(uint16 wServiceID);
		//设置接口
		virtual bool SetTCPSocketEvent(IUnknownEx * pIUnknownEx);

		//功能接口
	public:
		//关闭连接
		virtual bool CloseSocket();
		//连接地址
		virtual bool Connect(uint64 dwServerIP, uint16 wPort);
		//连接地址
		virtual bool Connect(std::string strServerIP, uint16 wPort);
		//发送函数
		virtual bool SendData(uint16 wMainCmdID, uint16 wSubCmdID, void * pData = nullptr, uint16 wDataSize = 0);

		//辅助函数
	protected:
		//连接消息
		bool OnSocketLink(int nErrorCode);
	};
}

#endif