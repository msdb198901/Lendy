#include "MessageBuffer.h"

namespace Util
{
	MessageBuffer::MessageBuffer() :
		m_wPos(0),
		m_rPos(0),
		m_storage()
	{
		m_storage.resize(4096);
	}

	MessageBuffer::MessageBuffer(std::size_t initalSize) :
		m_wPos(0),
		m_rPos(0),
		m_storage()
	{
		m_storage.resize(initalSize);
	}

	MessageBuffer::MessageBuffer(MessageBuffer const & right) :
		m_wPos(right.m_wPos),
		m_rPos(right.m_rPos),
		m_storage(right.m_storage)
	{
	}

	MessageBuffer::MessageBuffer(MessageBuffer && right) :
		m_wPos(right.m_wPos),
		m_rPos(right.m_rPos),
		m_storage(right.Move())
	{
	}

	void MessageBuffer::Reset()
	{
		m_wPos = 0;
		m_rPos = 0;
	}

	void MessageBuffer::Resize(size_type bytes)
	{
		m_storage.resize(bytes);
	}

	void MessageBuffer::Normalize()
	{
		if (m_rPos)
		{
			if (m_rPos != m_wPos)
			{
				memmove(GetBasePointer(), GetReadPointer(), GetActiveSize());
			}
			m_wPos -= m_rPos;
			m_rPos = 0;
		}
	}

	void MessageBuffer::EnsureFreeSpace()
	{
		if (GetRemainingSpace() == 0)
		{
			m_storage.resize(m_storage.size() * 3 / 2);
		}
	}

	void MessageBuffer::Write(void const * data, std::size_t size)
	{
		if (size)
		{
			memcpy(GetWritePointer(), data, size);
			WriteCompleted(size);
		}
	}

	std::vector<uint8>&& MessageBuffer::Move()
	{
		m_wPos = 0;
		m_rPos = 0;
		return std::move(m_storage);
	}

	MessageBuffer & MessageBuffer::operator=(MessageBuffer const & right)
	{
		if (this != &right)
		{
			m_wPos = right.m_wPos;
			m_rPos = right.m_rPos;
			m_storage = right.m_storage;
		}

		return *this;
	}

	MessageBuffer & MessageBuffer::operator=(MessageBuffer && right)
	{
		if (this != &right)
		{
			m_wPos = right.m_wPos;
			m_rPos = right.m_rPos;
			m_storage = right.Move();
		}

		return *this;
	}
	
}