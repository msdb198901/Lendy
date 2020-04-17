#ifndef SERVICE_UNITS_H
#define SERVICE_UNITS_H


#include "KernelEngineHead.h"
#include "DBExports.h"
#include "IOContext.h"
#include "AttemperEngineSink.h"
#include "TimerEngine.h"
#include "Header.h"
#include "Strand.h"
#include "DataQueue.h"
#include <functional>
#include <thread>
#include <mutex>

namespace Game
{
	class ServiceUnits
	{
		//����״̬
		enum enServiceStatus
		{
			ServiceStatus_Stop,				//ֹͣ״̬
			ServiceStatus_Config,			//����״̬
			ServiceStatus_Run,				//����״̬
		};

	public:
		ServiceUnits();

	public:
		static ServiceUnits* GetInstance();

	public:
		//��������
		bool Start(Net::IOContext* ioContext, int argc, char** argv);

		//ֹͣ����
		bool Conclude();

		//�ڲ�����
	protected:
		void Run();

		//�ڲ�����
	protected:
		//��������
		bool ParserArgs(int argc, char** argv);
		//��������
		bool UpdateConfig();
		//�������
		bool InitializeService();
		//�����ں�
		bool StartKernelService(Net::IOContext*);

		//�ڲ�����
	private:
		//����״̬
		bool SetServiceStatus(enServiceStatus ServiceStatus);
		//���Ϳ���
		bool SendControlPacket(uint16 wControlID, void * pData, uint16 wDataSize);

	public:
		//Ͷ������
		bool PostControlRequest(uint16 wIdentifier, void * pData, uint16 wDataSize);

	protected:
		//������Ϣ
		bool OnUIControlRequest();

		//�º���
	private:
		std::function<bool()>				m_funcStartNetService;

	private:
		//״̬����
		enServiceStatus						m_ServiceStatus;

		tagSubGameInfo						m_SubGameInfo;
		tagGameServiceOption 				m_GameServiceOption;				//��������

		//�������
	public:
		CAttemperEngineSink					m_AttemperEngineSink;				//���ȹ���

	private:
		CTimerEngineHelper					m_TimerEngine;						//ʱ������
		CAttemperEngineHelper				m_AttemperEngine;					//��������
		CTCPNetworkEngineHelper				m_TCPNetworkEngine;
		CTCPSocketServiceHelper				m_TCPSocketService;					//����ͨѶ
		CGameServiceManagerHelper			m_GameServiceManager;				//��Ϸģ��

		//�ڲ��Q��
	private:
		std::mutex							m_mutex;
		Net::IOContext 						m_ioContext;
		Net::Strand*						m_pStrand;
		std::thread*						m_pThread;
		DataQueue							m_dataQueue;
	};
}

#define SrvUnitsMgr Game::ServiceUnits::GetInstance()

#endif