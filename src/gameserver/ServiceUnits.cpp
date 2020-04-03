#include "ServiceUnits.h"
#include "Log.h"
#include "INIReader.h"

namespace Game
{
	using namespace LogComm;
	using namespace Util;
	ServiceUnits * ServiceUnits::GetInstance()
	{
		static ServiceUnits* _instance = nullptr;
		if (_instance == nullptr)
		{
			_instance = new ServiceUnits();
		}
		return _instance;
	}

	ServiceUnits::ServiceUnits() :
		m_ServiceStatus(ServiceStatus_Stop),
		m_ioContext(1),
		m_pStrand(new Net::Strand(m_ioContext)),
		m_pThread(nullptr)
	{
	}

	bool ServiceUnits::Start(Net::IOContext* ioContext)
	{
		assert(m_ServiceStatus == ServiceStatus_Stop);
		if (m_ServiceStatus != ServiceStatus_Stop) return false;

		m_ServiceStatus = ServiceStatus_Config;

		if (m_pThread = new std::thread(&ServiceUnits::Run, this), m_pThread == nullptr)
		{
			assert(nullptr);
			return false;
		}

		UpdateConfig();

		if (!InitializeService())
		{
			Conclude();
			return false;
		}

		//启动内核
		if (!StartKernelService(ioContext))
		{
			Conclude();
			return false;
		}

		//设置函数
		m_funcStartNetService = [&]()
		{
			if (!m_TCPNetworkEngine->Start(ioContext))
			{
				return false;
			}
			return true;
		};

		SendControlPacket(SUC_LOAD_DB_GAME_LIST, NULL, 0);
		return true;
	}
	bool ServiceUnits::Conclude()
	{
		m_ServiceStatus = ServiceStatus_Stop;

		if (m_AttemperEngine.GetDLLInterface())
		{
			m_AttemperEngine->Stop();
		}

		if (m_TCPNetworkEngine.GetDLLInterface())
		{
			m_TCPNetworkEngine->Stop();
		}

		if (m_TCPSocketService.GetDLLInterface())
		{
			m_TCPSocketService->Stop();
		}
		return true;
	}

	void ServiceUnits::Run()
	{
		//事件通知
		asio::io_context::work work(m_ioContext);
		m_ioContext.run();
	}

	bool ServiceUnits::UpdateConfig()
	{
		//效验状态
		////assert(m_ServiceStatus == ServiceStatus_Stop);
		//if (m_ServiceStatus != ServiceStatus_Stop) return false;

		//配置模块
		///m_GameServiceOption = GameServiceOption;
		m_GameServiceManager.SetMoudleDLLCreate("RedBlack.dll", SUB_GAME_CREATE_NAME);
		return true;
	}

	bool ServiceUnits::InitializeService()
	{
		if ((m_AttemperEngine.GetDLLInterface() == nullptr) && (!m_AttemperEngine.CreateInstance()))
		{
			return false;
		}

		if ((m_TCPNetworkEngine.GetDLLInterface() == nullptr) && (!m_TCPNetworkEngine.CreateInstance()))
		{
			return false;
		}

		if ((m_TCPSocketService.GetDLLInterface() == nullptr) && (!m_TCPSocketService.CreateInstance()))
		{
			return false;
		}

		//游戏模块
		if ((m_GameServiceManager.GetDLLInterface() == nullptr) && (!m_GameServiceManager.CreateInstance()))
		{
			return false;
		}
		
		//组件接口
		IUnknownEx * pIAttemperEngine = m_AttemperEngine.GetDLLInterface();
		IUnknownEx * pITCPNetworkEngine = m_TCPNetworkEngine.GetDLLInterface();
		IUnknownEx * pIAttemperEngineSink = QUERY_OBJECT_INTERFACE(m_AttemperEngineSink, IUnknownEx);

		//内核组件
		if (m_TCPNetworkEngine->SetTCPNetworkEngineEvent(pIAttemperEngine) == false) return false;
		if (m_AttemperEngine->SetNetworkEngine(pITCPNetworkEngine) == false) return false;
		if (m_AttemperEngine->SetAttemperEngineSink(pIAttemperEngineSink)==false) return false;

		if (m_TCPSocketService->SetServiceID(NETWORK_CORRESPOND) == false) return false;
		if (m_TCPSocketService->SetTCPSocketEvent(pIAttemperEngine) == false) return false;

		m_AttemperEngineSink.m_pITCPNetworkEngine = m_TCPNetworkEngine.GetDLLInterface();
		m_AttemperEngineSink.m_pITCPSocketService = m_TCPSocketService.GetDLLInterface();
		m_AttemperEngineSink.m_pIGameServiceManager = m_GameServiceManager.GetDLLInterface();

		if (!m_TCPNetworkEngine->SetServiceParameter(
			sConfigMgr->Get("Net", "BindIP", "127.0.0.1"),
			sConfigMgr->GetInt32("Net", "Port", 8600),
			sConfigMgr->GetInt32("Net", "Threads", 4)))
		{
			return false;
		}

		return true;
	}
	bool ServiceUnits::StartKernelService(Net::IOContext* ioContext)
	{
		if (!m_AttemperEngine->Start(ioContext))
		{
			return false;
		}

		if (!m_TCPSocketService->Start(ioContext))
		{
			return false;
		}

		//读取DB（DB不采用网狐会定义很多变量，直接用回调函数）
		LogonDatabasePool.Start(LogonDatabasePool);

		return true;
	}

	bool ServiceUnits::SetServiceStatus(enServiceStatus ServiceStatus)
	{
		if (m_ServiceStatus != ServiceStatus)
		{
			//错误通知
			if ((m_ServiceStatus != ServiceStatus_Run) && (ServiceStatus == ServiceStatus_Stop))
			{
				LOG_INFO("server.logon", "服务启动失败");
			}

			//设置变量
			m_ServiceStatus = ServiceStatus;

			switch (m_ServiceStatus)
			{
				case ServiceStatus_Stop:	//停止状态
				{
					LOG_INFO("server.logon", "服务停止成功");
					break;
				}
				case ServiceStatus_Config:	//配置状态
				{
					LOG_INFO("server.logon", "正在初始化组件...");
					break;
				}
				case ServiceStatus_Run:	//服务状态
				{
					LOG_INFO("server.logon", "服务启动成功");
					break;
				}
			}
		}
		return true;
	}

	bool ServiceUnits::SendControlPacket(uint16 wControlID, void * pData, uint16 wDataSize)
	{
		//状态效验
		assert(m_AttemperEngine.GetDLLInterface() != nullptr);
		if (m_AttemperEngine.GetDLLInterface() == nullptr) return false;

		//发送控制
		return m_AttemperEngine->OnEventControl(wControlID, pData, wDataSize);
	}

	bool ServiceUnits::PostControlRequest(uint16 wIdentifier, void * pData, uint16 wDataSize)
	{
		{
			std::lock_guard<std::mutex>	 _lock(m_mutex);
			m_dataQueue.InsertData(wIdentifier, pData, wDataSize);
		}

		Net::post(m_ioContext, Net::bind_executor(*m_pStrand, [this, wIdentifier, pData, wDataSize]() {
			OnUIControlRequest();
		}));
		return true;
	}

	bool ServiceUnits::OnUIControlRequest()
	{
		tagDataHead DataHead;
		uint8 cbBuffer[SOCKET_TCP_BUFFER] = {};

		//提取数据
		std::lock_guard<std::mutex>	 _lock(m_mutex);
		if (m_dataQueue.DistillData(DataHead, cbBuffer, sizeof(cbBuffer)) == false)
		{
			assert(nullptr);
			return false;
		}

		//数据处理
		switch (DataHead.wIdentifier)
		{
			case UDC_LOAD_DB_LIST_RESULT:	//列表结果
			{
				//效验消息
				assert(DataHead.wDataSize == sizeof(ControlResult));
				if (DataHead.wDataSize != sizeof(ControlResult)) return 0;

				//变量定义
				ControlResult * pControlResult = (ControlResult *)cbBuffer;

				//失败处理
				if ((m_ServiceStatus != ServiceStatus_Run) && (pControlResult->cbSuccess == 0))
				{
					Conclude();
					return 0;
				}

				//成功处理
				if ((m_ServiceStatus != ServiceStatus_Run) && (pControlResult->cbSuccess == 1))
				{
					//连接协调
					SendControlPacket(SUC_CONNECT_CORRESPOND, NULL, 0);
				}
				return 0;
			}
			case UDC_CORRESPOND_RESULT:
			{
				//效验消息
				assert(DataHead.wDataSize == sizeof(ControlResult));
				if (DataHead.wDataSize != sizeof(ControlResult)) return 0;

				//变量定义
				ControlResult * pControlResult = (ControlResult *)cbBuffer;

				//失败处理
				if ((m_ServiceStatus != ServiceStatus_Run) && (pControlResult->cbSuccess == 0))
				{
					Conclude();
					return 0;
				}

				//成功处理
				if ((m_ServiceStatus != ServiceStatus_Run) && (pControlResult->cbSuccess == 1))
				{
					//启动网络
					if (!m_funcStartNetService())
					{
						Conclude();
						return 0;
					}

					//设置状态
					SetServiceStatus(ServiceStatus_Run);
				}

				return 0;
			}
		}

		return true;
	}
}