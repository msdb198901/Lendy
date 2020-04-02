#include "DataQueue.h"
#include <memory>
#include <algorithm>

namespace Util
{
	DataQueue::DataQueue() : 
		m_dwInsertIndex(0),
		m_dwQueryIndex(0),
		m_dwTailIndex(0),
		m_dwQueueSize(0),
		m_dwDataSize(0),
		m_dwDataPacketCount(0),
		m_pQueueBuffer(nullptr)
	{
	}

	DataQueue::~DataQueue()
	{
		PDELETE(m_pQueueBuffer);
	}

	bool DataQueue::InsertData(uint16 wIdentifier, void * pBuffer, uint16 wDataSize)
	{
		//变量定义
		tagDataHead DataHead;
		memset(&DataHead, 0, sizeof(DataHead));

		//设置变量
		DataHead.wDataSize = wDataSize;
		DataHead.wIdentifier = wIdentifier;

		//调整存储
		if (RectifyBuffer(sizeof(DataHead) + DataHead.wDataSize) == false)
		{
			assert(nullptr);
			return false;
		}

		//插入数据
		try
		{
			memcpy(m_pQueueBuffer + m_dwInsertIndex, &DataHead, sizeof(DataHead));
			
			//附加数据
			if (wDataSize > 0)
			{
				assert(pBuffer != NULL);
				memcpy(m_pQueueBuffer + m_dwInsertIndex + sizeof(DataHead), pBuffer, wDataSize);
			}

			//调整数据
			++m_dwDataPacketCount;
			m_dwInsertIndex += sizeof(DataHead) + wDataSize;
			m_dwDataSize += sizeof(DataHead) + wDataSize;
			if (m_dwTailIndex < m_dwInsertIndex) m_dwTailIndex = m_dwInsertIndex;
			return true;
		}
		catch (...)
		{
			assert(pBuffer != NULL);
			return false;
		}
		return false;
	}

	bool DataQueue::DistillData(tagDataHead & DataHead, void * pBuffer, uint16 wBufferSize)
	{
		//效验变量
		assert(m_dwDataSize > 0L);
		assert(m_dwDataPacketCount > 0);
		assert(m_pQueueBuffer != NULL);

		//效验变量
		if (m_dwDataSize == 0) return false;
		if (m_dwDataPacketCount == 0) return false;

		//调整参数
		if (m_dwQueryIndex == m_dwTailIndex)
		{
			m_dwQueryIndex = 0;
			m_dwTailIndex = m_dwInsertIndex;
		}

		assert(m_dwQueueSize >= (m_dwQueryIndex + sizeof(tagDataHead)));
		tagDataHead * pDataHead = (tagDataHead *)(m_pQueueBuffer + m_dwQueryIndex);
		assert(wBufferSize >= pDataHead->wDataSize);

		//获取大小
		uint16 wPacketSize = sizeof(DataHead) + pDataHead->wDataSize;
		assert(m_dwQueueSize >= (m_dwQueryIndex + wPacketSize));

		//拷贝数据
		DataHead = *pDataHead;
		if (DataHead.wDataSize > 0)
		{
			if (wBufferSize < pDataHead->wDataSize) DataHead.wDataSize = 0;
			else memcpy(pBuffer, pDataHead + 1, DataHead.wDataSize);
		}

		//效验参数
		assert(wPacketSize <= m_dwDataSize);

		//设置变量
		m_dwDataPacketCount--;
		m_dwDataSize -= wPacketSize;
		m_dwQueryIndex += wPacketSize;
		return true;
	}

	bool DataQueue::RectifyBuffer(uint64 dwNeedSize)
	{
		//				0			    m_dwInsert				  m_dwQueueSize		
		//				|				     |		        		     |
		//				|____________________|___________________________|
		//				|	/	/	/	/	/	/	/	/	/	/	/   /|
		//				|__/___/___/___/___/___/___/___/___/___/___/___/_|
		//								|					|			
		//							m_dwQuery			m_dwQuery
		///////////////////////////////////////////////////////////////////////////
		bool bAlloc = false;

		//缓冲判断
		if ((m_dwDataSize + dwNeedSize) > m_dwQueueSize) bAlloc = true;

		//重新开始
		if ((m_dwInsertIndex == m_dwTailIndex) && ((m_dwInsertIndex + dwNeedSize) > m_dwQueueSize))
		{
			if (m_dwQueryIndex >= dwNeedSize) m_dwInsertIndex = 0;
			else bAlloc = true;
		}

		//缓冲判断
		if ((m_dwInsertIndex < m_dwTailIndex) && ((m_dwInsertIndex + dwNeedSize) > m_dwQueryIndex)) bAlloc = true;
		////////////////////////////////////////////////////  
		//头追上尾或尾追上头  
		if (m_dwInsertIndex + dwNeedSize > m_dwQueryIndex && m_dwQueryIndex >= m_dwInsertIndex)
		{
			//尾追上头  
			if (m_dwDataSize > 0) bAlloc = true;
		}

		try
		{
			if (bAlloc)
			{
				//申请内存
				uint64 dwReSize = __max(m_dwQueueSize / 2L, dwNeedSize * 10L);
				uint8* pNewQueueBuffer = new uint8[m_dwQueueSize + dwReSize];

				//错误判断
				assert(pNewQueueBuffer != nullptr);
				if (pNewQueueBuffer == nullptr) return false;

				//拷贝数据
				uint64 dwRemainSize = 0;
				if (m_pQueueBuffer != nullptr)
				{
					dwRemainSize = m_dwTailIndex - m_dwQueryIndex;
					if (dwRemainSize > 0) memcpy(pNewQueueBuffer, m_pQueueBuffer + m_dwQueryIndex, dwRemainSize);
					if (m_dwInsertIndex < m_dwQueryIndex)
					{
						memcpy(pNewQueueBuffer + dwRemainSize, m_pQueueBuffer, m_dwQueryIndex);
						dwRemainSize += m_dwQueryIndex;
					}
				}

				//设置变量
				m_dwQueryIndex = 0;
				m_dwInsertIndex = dwRemainSize;
				m_dwTailIndex = dwRemainSize;
				m_dwQueueSize = m_dwQueueSize + dwReSize;

				//设置缓冲
				PDELETE(m_pQueueBuffer);
				m_pQueueBuffer = pNewQueueBuffer;
			}
		}
		catch (...) { return false; }


		//try
		//{
		//	//
		//	if (m_dwInsert > m_IndexQuery.dwIndex)
		//	{
		//		if (m_dwInsert + dwNeedSize > m_dwQueueSize)
		//		{
		//			if (m_IndexQuery.dwIndex > dwNeedSize)
		//			{
		//				m_dwInsert = 0;
		//			}
		//			else
		//			{
		//				throw 0;
		//			}
		//		}
		//	}
		//	else
		//	{
		//		if (m_dwInsert + dwNeedSize > m_IndexQuery.dwIndex)
		//		{
		//			throw 0;
		//		}
		//	}
		//}
		//catch (...)
		//{
		//	try
		//	{
		//		//申请内存
		//		uint64 dwReSize = __max(m_dwQueueSize / 2L, dwNeedSize * 10L);
		//		uint8* pNewQueueBuffer = new uint8[m_dwQueueSize + dwReSize];

		//		//错误判断
		//		assert(pNewQueueBuffer != nullptr);
		//		if (pNewQueueBuffer == nullptr) return false;

		//		//拷贝数据
		//		uint64 dwRemainSize = 0;
		//		if (m_pQueueBuffer != NULL)
		//		{
		//			dwRemainSize = m_dwQueueSize - m_IndexQuery.dwIndex;
		//			memcpy(pNewQueueBuffer, m_pQueueBuffer + m_IndexQuery.dwIndex, m_dwQueueSize - m_IndexQuery.dwIndex);
		//			if (m_dwInsert < m_IndexQuery.dwIndex)
		//			{
		//				memcpy(pNewQueueBuffer + m_dwQueueSize - m_IndexQuery.dwIndex, m_pQueueBuffer, m_dwInsert);
		//				dwRemainSize += m_dwInsert;
		//			}
		//		}

		//		//设置变量
		//		memset(&m_IndexQuery, 0, sizeof(m_IndexQuery));
		//		m_dwInsert = dwRemainSize;
		//		m_dwQueueSize = m_dwQueueSize + dwReSize;

		//		uint16 uLastSize = 0;
		//		std::deque<tagDataIndex> d;
		//		for (std::deque<tagDataIndex>::iterator it = m_deque.begin(); it != m_deque.end(); ++it)
		//		{
		//			if (it->dwIndex > m_deque.begin()->dwIndex)
		//			{
		//				it->dwIndex -= m_deque.begin()->dwIndex;
		//				uLastSize = it->dwIndex + it->wDataSize;
		//			}
		//			else
		//			{
		//				it->dwIndex += uLastSize;
		//			}
		//			d.emplace_back(*it);
		//		}
		//		m_deque.assign(d.begin(),d.end());

		//		//设置缓冲
		//		PDELETE(m_pQueueBuffer);
		//		m_pQueueBuffer = pNewQueueBuffer;
		//	}
		//	catch (...) { return false; }
		//}
		return true;
	}
}