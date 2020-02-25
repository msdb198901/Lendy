#include "BigNumber.h"
#include <openssl/bn.h>
#include <string>
#include <algorithm>
#include <memory>

namespace Util
{
	BigNumber::BigNumber() : m_bn(BN_new())
	{
	}
	BigNumber::BigNumber(BigNumber const & bn) : m_bn(BN_dup(bn.m_bn))
	{
	}
	BigNumber::BigNumber(uint32 val) : m_bn(BN_new())
	{
		BN_set_word(m_bn, val);
	}
	BigNumber::~BigNumber()
	{
		BN_free(m_bn);
	}

	void BigNumber::SetDWord(uint32 val)
	{
		BN_set_word(m_bn, val);
	}

	void BigNumber::SetQWord(uint64 val)
	{
		BN_set_word(m_bn, (uint32)(val >> 32));
		BN_lshift(m_bn, m_bn, 32);
		BN_add_word(m_bn, (uint32)(val & 0xFFFFFFFF));
	}

	void BigNumber::SetBinary(uint8 const* bytes, int32 len)
	{
		uint8* array = new uint8[len];

		for (int i = 0; i < len; i++)
			array[i] = bytes[len - 1 - i];

		BN_bin2bn(array, len, m_bn);

		delete[] array;
	}

	bool BigNumber::SetHexStr(char const* str)
	{
		int n = BN_hex2bn(&m_bn, str);
		return (n > 0);
	}

	void BigNumber::SetRand(int32 numbits)
	{
		BN_rand(m_bn, numbits, 0, 1);
	}

	BigNumber& BigNumber::operator=(BigNumber const& bn)
	{
		if (this == &bn)
			return *this;

		BN_copy(m_bn, bn.m_bn);
		return *this;
	}

	BigNumber& BigNumber::operator+=(BigNumber const& bn)
	{
		BN_add(m_bn, m_bn, bn.m_bn);
		return *this;
	}

	BigNumber& BigNumber::operator-=(BigNumber const& bn)
	{
		BN_sub(m_bn, m_bn, bn.m_bn);
		return *this;
	}

	BigNumber& BigNumber::operator*=(BigNumber const& bn)
	{
		BN_CTX *bnctx;

		bnctx = BN_CTX_new();
		BN_mul(m_bn, m_bn, bn.m_bn, bnctx);
		BN_CTX_free(bnctx);

		return *this;
	}

	BigNumber& BigNumber::operator/=(BigNumber const& bn)
	{
		BN_CTX *bnctx;

		bnctx = BN_CTX_new();
		BN_div(m_bn, nullptr, m_bn, bn.m_bn, bnctx);
		BN_CTX_free(bnctx);

		return *this;
	}

	BigNumber& BigNumber::operator%=(BigNumber const& bn)
	{
		BN_CTX *bnctx;

		bnctx = BN_CTX_new();
		BN_mod(m_bn, m_bn, bn.m_bn, bnctx);
		BN_CTX_free(bnctx);

		return *this;
	}

	BigNumber& BigNumber::operator<<=(int n)
	{
		BN_lshift(m_bn, m_bn, n);
		return *this;
	}

	int BigNumber::CompareTo(BigNumber const& bn) const
	{
		return BN_cmp(m_bn, bn.m_bn);
	}

	BigNumber BigNumber::Exp(BigNumber const& bn) const
	{
		BigNumber ret;
		BN_CTX *bnctx;

		bnctx = BN_CTX_new();
		BN_exp(ret.m_bn, m_bn, bn.m_bn, bnctx);
		BN_CTX_free(bnctx);

		return ret;
	}

	BigNumber BigNumber::ModExp(BigNumber const& bn1, BigNumber const& bn2) const
	{
		BigNumber ret;
		BN_CTX *bnctx;

		bnctx = BN_CTX_new();
		BN_mod_exp(ret.m_bn, m_bn, bn1.m_bn, bn2.m_bn, bnctx);
		BN_CTX_free(bnctx);

		return ret;
	}

	int32 BigNumber::GetNumBytes() const
	{
		return BN_num_bytes(m_bn);
	}

	uint32 BigNumber::AsDWord() const
	{
		return (uint32)BN_get_word(m_bn);
	}

	bool BigNumber::IsZero() const
	{
		return BN_is_zero(m_bn);
	}

	bool BigNumber::IsNegative() const
	{
		return BN_is_negative(m_bn);
	}

	bool BigNumber::AsByteArray(uint8* buf, std::size_t bufsize, bool littleEndian) const
	{
		int nBytes = GetNumBytes();
		assert(!(nBytes < 0));
		std::size_t numBytes = static_cast<std::size_t>(nBytes);

		// too large to store
		if (bufsize < numBytes)
			return false;

		// If we need more bytes than length of BigNumber set the rest to 0
		if (numBytes < bufsize)
			memset((void*)buf, 0, bufsize);

		BN_bn2bin(m_bn, buf + (bufsize - numBytes));

		// openssl's BN stores data internally in big endian format, reverse if little endian desired
		if (littleEndian)
			std::reverse(buf, buf + bufsize);

		return true;
	}

	std::unique_ptr<uint8[]> BigNumber::AsByteArray(int32 minSize, bool littleEndian) const
	{
		std::size_t length = std::max(GetNumBytes(), minSize);
		uint8* array = new uint8[length];
		bool success = AsByteArray(array, length, littleEndian);
		assert(success);

		return std::unique_ptr<uint8[]>(array);
	}

	std::string BigNumber::AsHexStr() const
	{
		char* ch = BN_bn2hex(m_bn);
		std::string ret = ch;
		OPENSSL_free(ch);
		return ret;
	}

	std::string BigNumber::AsDecStr() const
	{
		char* ch = BN_bn2dec(m_bn);
		std::string ret = ch;
		OPENSSL_free(ch);
		return ret;
	}
}
