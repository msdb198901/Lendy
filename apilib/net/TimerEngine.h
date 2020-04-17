#ifndef TIMER_ENGINE_H
#define TIMER_ENGINE_H

#include "KernelEngineHead.h"
#include "Strand.h"
#include <map>
#include <thread>
#include <mutex>

namespace Net
{
#define NO_TIME_LEAVE	uint32(-1)
#define TIMES_INFINITY	NO_TIME_LEAVE

	//定时器子项
	struct tagTimerItem
	{
		//时间参数
		uint32							dwElapse;							//倒数时间
		uint32							dwTimerID;							//时间标识
		uint32							dwTimeLeave;						//剩余时间
		uint32							dwRepeatTimes;						//重复次数
	};

	typedef std::vector<tagTimerItem*>		CTimerItemArray;
	typedef std::map<uint32, tagTimerItem*>	CTimerItemStore;

	class CTimerEngine : public ITimerEngine
	{
		//函数定义
	public:
		//构造函数
		CTimerEngine();
		//析构函数
		virtual ~CTimerEngine();

		//基础接口
	public:
		//释放对象
		virtual void Release();
		//接口查询
		virtual void *QueryInterface(GGUID uuid);
		
		//服务接口
	public:
		//启动服务
		virtual bool Start(Net::IOContext*);
		//停止服务
		virtual bool Stop();

		//功能接口
	public:
		//设置定时器
		virtual bool SetTimer(uint32 dwTimerID, uint32 dwElapse, uint32 dwRepeat);
		//删除定时器
		virtual bool KillTimer(uint32 dwTimerID);
		//删除定时器
		virtual bool KillAllTimer();

		//内部调度
	private:
		void Run();

		void Loop();

		//内部函数
	private:
		//定时器通知
		void OnTimerThreadSink();

		//配置接口
	public:
		//设置接口
		virtual bool SetTimerEngineEvent(IUnknownEx * pIUnknownEx);

	protected:
		uint32								m_dwCount;
		uint32								m_dwTimerSpace;
		uint32								m_dwTimePass;						//经过时间
		uint32								m_dwTimeLeave;
		uint32								m_dwLastTickCount;
		uint32								m_dwShortTime;

		ITimerEngineEvent *					m_pITimerEngineEvent;				//事件接口
		CTimerItemArray						m_TimerItemFree;					//空闲数组
		CTimerItemStore						m_TimerItemActive;					//空闲数组

	protected:
		std::shared_ptr<Net::IOContext> 	m_ioContext;
		int									m_iCount;
		std::mutex							m_mutex;
		std::thread*						m_pThread;
		std::thread*						m_pThreadLoop;
		asio::steady_timer					m_updateTimer;
	};
}

#endif