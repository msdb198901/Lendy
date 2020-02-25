#ifndef SHA1_H
#define SHA1_H

#include "Define.h"
#include <string>
#include <openssl/sha.h>

namespace Util
{
	class BigNumber;

	class LENDY_COMMON_API SHA1Hash
	{
	public:
		SHA1Hash();
		~SHA1Hash();

		void UpdateBigNumbers(BigNumber* bn0, ...);

		void UpdateData(const uint8 *dta, int len);
		void UpdateData(const std::string &str);

		void Initialize();
		void Finalize();

		uint8 *GetDigest(void) { return mDigest; }
		int GetLength(void) const { return SHA_DIGEST_LENGTH; }

	private:
		SHA_CTX mC;
		uint8 mDigest[SHA_DIGEST_LENGTH];
	};

	LENDY_COMMON_API std::string CalculateSHA1Hash(std::string const& content);
}

#endif