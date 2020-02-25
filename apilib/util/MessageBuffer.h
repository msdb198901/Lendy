#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#include "Define.h"
#include <vector>
#include <cstring>

namespace Util
{
	class LENDY_COMMON_API MessageBuffer
	{
		typedef std::vector<uint8>::size_type size_type;

	public:
		MessageBuffer();

		explicit MessageBuffer(std::size_t initalSize);

		MessageBuffer(MessageBuffer const& right);

		MessageBuffer(MessageBuffer&& right);

		void Reset();

		void Resize(size_type bytes);

		uint8* GetBasePointer() { return m_storage.data(); }

		uint8* GetReadPointer() { return GetBasePointer() + m_rPos; }

		uint8* GetWritePointer() { return GetBasePointer() + m_wPos; }

		void ReadCompleted(size_type bytes) { m_rPos += bytes; }

		void WriteCompleted(size_type bytes) { m_wPos += bytes; }

		size_type GetActiveSize() const { return m_wPos - m_rPos; }

		size_type GetRemainingSpace() const { return m_storage.size() - m_wPos; }

		size_type GetBufferSize() const { return m_storage.size(); }

		void Normalize();

		void EnsureFreeSpace();

		void Write(void const* data, std::size_t size);

		std::vector<uint8>&& Move();

		MessageBuffer& operator=(MessageBuffer const& right);

		MessageBuffer& operator=(MessageBuffer&& right);

	private:
		size_type			m_wPos;
		size_type			m_rPos;

		EXPORT_BEGIN
		std::vector<uint8>	m_storage;
		EXPORT_END
	};
}

#endif