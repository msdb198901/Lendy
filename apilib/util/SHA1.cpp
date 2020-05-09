#include "SHA1.h"
#include "BigNumber.h"
#include "StringUtility.h"
#include <cstring>
#include <cstdarg>
#include <sstream>

namespace Util
{
	SHA1Hash::SHA1Hash()
	{
		SHA1_Init(&mC);
		memset(mDigest, 0, SHA_DIGEST_LENGTH * sizeof(uint8));
	}

	SHA1Hash::~SHA1Hash()
	{
		SHA1_Init(&mC);
	}

	void SHA1Hash::UpdateData(const uint8 *dta, int len)
	{
		SHA1_Update(&mC, dta, len);
	}

	void SHA1Hash::UpdateData(const std::string &str)
	{
		UpdateData((uint8 const*)str.c_str(), str.length());
	}

	void SHA1Hash::UpdateBigNumbers(BigNumber* bn0, ...)
	{
		va_list v;
		BigNumber* bn;

		va_start(v, bn0);
		bn = bn0;
		while (bn)
		{
			UpdateData(bn->AsByteArray().get(), bn->GetNumBytes());
			bn = va_arg(v, BigNumber*);
		}
		va_end(v);
	}

	void SHA1Hash::Initialize()
	{
		SHA1_Init(&mC);
	}

	void SHA1Hash::Finalize(void)
	{
		SHA1_Final(mDigest, &mC);
	}

	std::string CalculateSHA1Hash(std::string const& content)
	{
		unsigned char digest[SHA_DIGEST_LENGTH];
		SHA1((unsigned char*)content.c_str(), content.length(), (unsigned char*)&digest);

		auto ByteArrayToHexStr = [](uint8 const* bytes, uint32 arrayLen, bool reverse = false)
		{
			int32 init = 0;
			int32 end = arrayLen;
			int8 op = 1;

			if (reverse)
			{
				init = arrayLen - 1;
				end = -1;
				op = -1;
			}

			std::ostringstream ss;
			for (int32 i = init; i != end; i += op)
			{
				char buffer[4];
				sprintf(buffer, "%02X", bytes[i]);
				ss << buffer;
			}

			return ss.str();
		};
		return ByteArrayToHexStr(digest, SHA_DIGEST_LENGTH);
	}
}



