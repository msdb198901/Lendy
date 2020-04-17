#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include "Define.h"
#include <deque>

namespace Util
{
	//数据列头
	struct tagDataHead
	{
		uint16							wDataSize;							//数据大小
		uint16							wIdentifier;						//类型标识
	};

	class LENDY_COMMON_API DataQueue
	{
		//查询变量
	protected:
		uint32							m_dwInsertPos;					//插入位置
		uint32							m_dwTerminalPos;				//结束位置
		uint32							m_dwDataQueryPos;				//查询位置

		//数据变量
	protected:
		uint32							m_dwDataSize;					//数据大小
		uint32							m_dwDataPacketCount;			//数据包数
				
		//缓冲变量
	protected:
		uint32							m_dwBufferSize;					//缓冲长度
		uint8*							m_pDataQueueBuffer;				//缓冲指针

		//函数定义
	public:
		//构造函数
		DataQueue();
		//析构函数
		virtual ~DataQueue();

		//插入数据
	public:
		//插入数据
		bool InsertData(uint16 wIdentifier, void * pBuffer, uint16 wDataSize);

		//提取数据
		bool DistillData(tagDataHead & DataHead, void * pBuffer, uint16 wBufferSize);

		//内部函数
	private:
		//调整存储
		bool RectifyBuffer(uint64 dwNeedSize);
	};
}


#endif