#ifndef BIGNUMBER_H
#define BIGNUMBER_H

#include "Define.h"
#include <array>
#include <memory>
#include <string>

struct bignum_st;

namespace Util
{
	class LENDY_COMMON_API BigNumber
	{
	public:
		BigNumber();
		BigNumber(BigNumber const& bn);
		BigNumber(uint32);
		~BigNumber();

		void SetDWord(uint32);
		void SetQWord(uint64);
		void SetBinary(uint8 const* bytes, int32 len);
		bool SetHexStr(char const* str);

		void SetRand(int32 numbits);

		BigNumber& operator=(BigNumber const& bn);

		BigNumber& operator+=(BigNumber const& bn);
		BigNumber operator+(BigNumber const& bn) const
		{
			BigNumber t(*this);
			return t += bn;
		}

		BigNumber& operator-=(BigNumber const& bn);
		BigNumber operator-(BigNumber const& bn) const
		{
			BigNumber t(*this);
			return t -= bn;
		}

		BigNumber& operator*=(BigNumber const& bn);
		BigNumber operator*(BigNumber const& bn) const
		{
			BigNumber t(*this);
			return t *= bn;
		}

		BigNumber& operator/=(BigNumber const& bn);
		BigNumber operator/(BigNumber const& bn) const
		{
			BigNumber t(*this);
			return t /= bn;
		}

		BigNumber& operator%=(BigNumber const& bn);
		BigNumber operator%(BigNumber const& bn) const
		{
			BigNumber t(*this);
			return t %= bn;
		}

		BigNumber& operator<<=(int n);
		BigNumber operator<<(int n) const
		{
			BigNumber t(*this);
			return t <<= n;
		}

		int CompareTo(BigNumber const& bn) const;
		bool operator<=(BigNumber const& bn) const { return (CompareTo(bn) <= 0); }
		bool operator==(BigNumber const& bn) const { return (CompareTo(bn) == 0); }
		bool operator>=(BigNumber const& bn) const { return (CompareTo(bn) >= 0); }
		bool operator<(BigNumber const& bn) const { return (CompareTo(bn) < 0); }
		bool operator>(BigNumber const& bn) const { return (CompareTo(bn) > 0); }

		bool IsZero() const;
		bool IsNegative() const;

		BigNumber ModExp(BigNumber const& bn1, BigNumber const& bn2) const;
		BigNumber Exp(BigNumber const&) const;

		int32 GetNumBytes() const;

		struct bignum_st *BN() { return m_bn; }

		uint32 AsDWord() const;

		bool AsByteArray(uint8* buf, std::size_t bufsize, bool littleEndian = true) const;
		std::unique_ptr<uint8[]> AsByteArray(int32 minSize = 0, bool littleEndian = true) const;
		template <std::size_t N> std::array<uint8, N> AsByteArray(bool littleEndian = true) const
		{
			std::array<uint8, N> buf;
			AsByteArray(buf.data(), N, littleEndian);
			return buf;
		}

		std::string AsHexStr() const;
		std::string AsDecStr() const;

	private:
		struct bignum_st *m_bn;
	};
}

#endif