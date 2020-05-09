#ifndef STRING_UTILITY_H_
#define STRING_UTILITY_H_

#include "Define.h"
#include <string>
#include <vector>

namespace Util 
{
	class LENDY_COMMON_API StringUtility
	{
	public:
		static void Split(const std::string& str, const std::string& delim, std::vector<std::string>* result);

		static bool StartWith(const std::string& str, const std::string& prefix);

		static bool EndsWith(const std::string& str, const std::string& suffix);

		static std::string& Ltrim(std::string& str);

		static std::string& Rtrim(std::string& str);

		static std::string& Trim(std::string& str);

		static void Trim(std::vector<std::string>* str_list);

		static void string_replace(const std::string &sub_str1, const std::string &sub_str2, std::string *str);

		static void UrlEncode(const std::string& src_str, std::string* dst_str);

		static void UrlDecode(const std::string& src_str, std::string* dst_str);

		static void ToUpper(std::string* str);

		static void ToLower(std::string* str);

		static bool StripSuffix(std::string* str, const std::string& suffix);

		static bool StripPrefix(std::string* str, const std::string& prefix);

		static bool Hex2Bin(const char* hex_str, std::string* bin_str);

		static bool Bin2Hex(const char* bin_str, std::string* hex_str);

		//////////////////////////////////////////////////////////
		//Ansi <-> Unicode <-> Utf8
		static void UTF8Printf(FILE* out, const char *str, ...);

		static void VUTF8Printf(FILE* out, const char *str, va_list* ap);

		static bool IsBasicLatinCharacter(wchar_t wchar);

		static wchar_t wcharToUpper(wchar_t wchar);
	
		static wchar_t wcharToUpperOnlyLatin(wchar_t wchar);
		
		static bool WStrToUtf8(std::wstring const& wstr, std::string& utf8str);

		static bool Utf8ToWStr(const std::string& utf8str, std::wstring& wstr);
		
		static bool Utf8ToUpperOnlyLatin(std::string& utf8String);
		
		static std::wstring StringToWString(const std::string& str);

		static std::string WStringToString(const std::wstring& wtr);

		static bool ConsoleToUtf8(const std::string& conStr, std::string& utf8str);

		static bool Utf8ToConsole(const std::string& utf8str, std::string& conStr);

		static struct tm* localtime_r(time_t const* time, struct tm *result);
	};
	

	class LENDY_COMMON_API Tokenizer
	{
	public:
		typedef std::vector<char const*> StorageType;

		typedef StorageType::size_type size_type;
		typedef StorageType::const_iterator const_iterator;
		typedef StorageType::reference reference;
		typedef StorageType::const_reference const_reference;

	public:
		Tokenizer(const std::string &src, char const sep, uint32 vectorReserve = 0, bool keepEmptyStrings = true);
		~Tokenizer() { delete[] m_str; }

		const_iterator begin() const { return m_storage.begin(); }
		const_iterator end() const { return m_storage.end(); }

		size_type size() const { return m_storage.size(); }

		reference operator [] (size_type i) { return m_storage[i]; }
		const_reference operator [] (size_type i) const { return m_storage[i]; }
	
	private:
		char* m_str;
		EXPORT_BEGIN
		StorageType m_storage;
		EXPORT_END
	};
} 
#endif
