#include "StringUtility.h"
#include <string>
#include <functional> 
#include <algorithm>
#include <cstdarg>
#include <ctime>
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS 
#include <Windows.h>
#endif

#include <utf8.h>

namespace Util 
{

void StringUtility::Split(const std::string& str,
    const std::string& delim,
    std::vector<std::string>* result) {
    if (str.empty()) {
        return;
    }
    if (delim[0] == '\0') {
        result->push_back(str);
        return;
    }

    size_t delim_length = delim.length();

    for (std::string::size_type begin_index = 0; begin_index < str.size();) {
        std::string::size_type end_index = str.find(delim, begin_index);
        if (end_index == std::string::npos) {
            result->push_back(str.substr(begin_index));
            return;
        }
        if (end_index > begin_index) {
            result->push_back(str.substr(begin_index, (end_index - begin_index)));
        }

        begin_index = end_index + delim_length;
    }
}

bool StringUtility::StartWith(const std::string& str, const std::string& prefix) {
    if (prefix.length() > str.length()) {
        return false;
    }

    if (memcmp(str.c_str(), prefix.c_str(), prefix.length()) == 0) {
        return true;
    }

    return false;
}

bool StringUtility::EndsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }

    return (str.substr(str.length() - suffix.length()) == suffix);
}

std::string& StringUtility::Ltrim(std::string& str) { // NOLINT
    std::string::iterator it = find_if(str.begin(), str.end(), std::not1(std::ptr_fun(::isspace)));
    str.erase(str.begin(), it);
    return str;
}

std::string& StringUtility::Rtrim(std::string& str) { // NOLINT
    std::string::reverse_iterator it = find_if(str.rbegin(),
        str.rend(), std::not1(std::ptr_fun(::isspace)));

    str.erase(it.base(), str.end());
    return str;
}

std::string& StringUtility::Trim(std::string& str) { // NOLINT
    return Rtrim(Ltrim(str));
}

void StringUtility::Trim(std::vector<std::string>* str_list) {
    if (nullptr == str_list) {
        return;
    }

    std::vector<std::string>::iterator it;
    for (it = str_list->begin(); it != str_list->end(); ++it) {
        *it = Trim(*it);
    }
}

void StringUtility::string_replace(const std::string &sub_str1,
    const std::string &sub_str2, std::string *str) {
    std::string::size_type pos = 0;
    std::string::size_type a = sub_str1.size();
    std::string::size_type b = sub_str2.size();
    while ((pos = str->find(sub_str1, pos)) != std::string::npos) {
        str->replace(pos, a, sub_str2);
        pos += b;
    }
}

static const char ENCODECHARS[1024] = {
    3, '%', '0', '0', 3, '%', '0', '1', 3, '%', '0', '2', 3, '%', '0', '3',
    3, '%', '0', '4', 3, '%', '0', '5', 3, '%', '0', '6', 3, '%', '0', '7',
    3, '%', '0', '8', 3, '%', '0', '9', 3, '%', '0', 'A', 3, '%', '0', 'B',
    3, '%', '0', 'C', 3, '%', '0', 'D', 3, '%', '0', 'E', 3, '%', '0', 'F',
    3, '%', '1', '0', 3, '%', '1', '1', 3, '%', '1', '2', 3, '%', '1', '3',
    3, '%', '1', '4', 3, '%', '1', '5', 3, '%', '1', '6', 3, '%', '1', '7',
    3, '%', '1', '8', 3, '%', '1', '9', 3, '%', '1', 'A', 3, '%', '1', 'B',
    3, '%', '1', 'C', 3, '%', '1', 'D', 3, '%', '1', 'E', 3, '%', '1', 'F',
    1, '+', '2', '0', 3, '%', '2', '1', 3, '%', '2', '2', 3, '%', '2', '3',
    3, '%', '2', '4', 3, '%', '2', '5', 3, '%', '2', '6', 3, '%', '2', '7',
    3, '%', '2', '8', 3, '%', '2', '9', 3, '%', '2', 'A', 3, '%', '2', 'B',
    3, '%', '2', 'C', 1, '-', '2', 'D', 1, '.', '2', 'E', 3, '%', '2', 'F',
    1, '0', '3', '0', 1, '1', '3', '1', 1, '2', '3', '2', 1, '3', '3', '3',
    1, '4', '3', '4', 1, '5', '3', '5', 1, '6', '3', '6', 1, '7', '3', '7',
    1, '8', '3', '8', 1, '9', '3', '9', 3, '%', '3', 'A', 3, '%', '3', 'B',
    3, '%', '3', 'C', 3, '%', '3', 'D', 3, '%', '3', 'E', 3, '%', '3', 'F',
    3, '%', '4', '0', 1, 'A', '4', '1', 1, 'B', '4', '2', 1, 'C', '4', '3',
    1, 'D', '4', '4', 1, 'E', '4', '5', 1, 'F', '4', '6', 1, 'G', '4', '7',
    1, 'H', '4', '8', 1, 'I', '4', '9', 1, 'J', '4', 'A', 1, 'K', '4', 'B',
    1, 'L', '4', 'C', 1, 'M', '4', 'D', 1, 'N', '4', 'E', 1, 'O', '4', 'F',
    1, 'P', '5', '0', 1, 'Q', '5', '1', 1, 'R', '5', '2', 1, 'S', '5', '3',
    1, 'T', '5', '4', 1, 'U', '5', '5', 1, 'V', '5', '6', 1, 'W', '5', '7',
    1, 'X', '5', '8', 1, 'Y', '5', '9', 1, 'Z', '5', 'A', 3, '%', '5', 'B',
    3, '%', '5', 'C', 3, '%', '5', 'D', 3, '%', '5', 'E', 1, '_', '5', 'F',
    3, '%', '6', '0', 1, 'a', '6', '1', 1, 'b', '6', '2', 1, 'c', '6', '3',
    1, 'd', '6', '4', 1, 'e', '6', '5', 1, 'f', '6', '6', 1, 'g', '6', '7',
    1, 'h', '6', '8', 1, 'i', '6', '9', 1, 'j', '6', 'A', 1, 'k', '6', 'B',
    1, 'l', '6', 'C', 1, 'm', '6', 'D', 1, 'n', '6', 'E', 1, 'o', '6', 'F',
    1, 'p', '7', '0', 1, 'q', '7', '1', 1, 'r', '7', '2', 1, 's', '7', '3',
    1, 't', '7', '4', 1, 'u', '7', '5', 1, 'v', '7', '6', 1, 'w', '7', '7',
    1, 'x', '7', '8', 1, 'y', '7', '9', 1, 'z', '7', 'A', 3, '%', '7', 'B',
    3, '%', '7', 'C', 3, '%', '7', 'D', 1, '~', '7', 'E', 3, '%', '7', 'F',
    3, '%', '8', '0', 3, '%', '8', '1', 3, '%', '8', '2', 3, '%', '8', '3',
    3, '%', '8', '4', 3, '%', '8', '5', 3, '%', '8', '6', 3, '%', '8', '7',
    3, '%', '8', '8', 3, '%', '8', '9', 3, '%', '8', 'A', 3, '%', '8', 'B',
    3, '%', '8', 'C', 3, '%', '8', 'D', 3, '%', '8', 'E', 3, '%', '8', 'F',
    3, '%', '9', '0', 3, '%', '9', '1', 3, '%', '9', '2', 3, '%', '9', '3',
    3, '%', '9', '4', 3, '%', '9', '5', 3, '%', '9', '6', 3, '%', '9', '7',
    3, '%', '9', '8', 3, '%', '9', '9', 3, '%', '9', 'A', 3, '%', '9', 'B',
    3, '%', '9', 'C', 3, '%', '9', 'D', 3, '%', '9', 'E', 3, '%', '9', 'F',
    3, '%', 'A', '0', 3, '%', 'A', '1', 3, '%', 'A', '2', 3, '%', 'A', '3',
    3, '%', 'A', '4', 3, '%', 'A', '5', 3, '%', 'A', '6', 3, '%', 'A', '7',
    3, '%', 'A', '8', 3, '%', 'A', '9', 3, '%', 'A', 'A', 3, '%', 'A', 'B',
    3, '%', 'A', 'C', 3, '%', 'A', 'D', 3, '%', 'A', 'E', 3, '%', 'A', 'F',
    3, '%', 'B', '0', 3, '%', 'B', '1', 3, '%', 'B', '2', 3, '%', 'B', '3',
    3, '%', 'B', '4', 3, '%', 'B', '5', 3, '%', 'B', '6', 3, '%', 'B', '7',
    3, '%', 'B', '8', 3, '%', 'B', '9', 3, '%', 'B', 'A', 3, '%', 'B', 'B',
    3, '%', 'B', 'C', 3, '%', 'B', 'D', 3, '%', 'B', 'E', 3, '%', 'B', 'F',
    3, '%', 'C', '0', 3, '%', 'C', '1', 3, '%', 'C', '2', 3, '%', 'C', '3',
    3, '%', 'C', '4', 3, '%', 'C', '5', 3, '%', 'C', '6', 3, '%', 'C', '7',
    3, '%', 'C', '8', 3, '%', 'C', '9', 3, '%', 'C', 'A', 3, '%', 'C', 'B',
    3, '%', 'C', 'C', 3, '%', 'C', 'D', 3, '%', 'C', 'E', 3, '%', 'C', 'F',
    3, '%', 'D', '0', 3, '%', 'D', '1', 3, '%', 'D', '2', 3, '%', 'D', '3',
    3, '%', 'D', '4', 3, '%', 'D', '5', 3, '%', 'D', '6', 3, '%', 'D', '7',
    3, '%', 'D', '8', 3, '%', 'D', '9', 3, '%', 'D', 'A', 3, '%', 'D', 'B',
    3, '%', 'D', 'C', 3, '%', 'D', 'D', 3, '%', 'D', 'E', 3, '%', 'D', 'F',
    3, '%', 'E', '0', 3, '%', 'E', '1', 3, '%', 'E', '2', 3, '%', 'E', '3',
    3, '%', 'E', '4', 3, '%', 'E', '5', 3, '%', 'E', '6', 3, '%', 'E', '7',
    3, '%', 'E', '8', 3, '%', 'E', '9', 3, '%', 'E', 'A', 3, '%', 'E', 'B',
    3, '%', 'E', 'C', 3, '%', 'E', 'D', 3, '%', 'E', 'E', 3, '%', 'E', 'F',
    3, '%', 'F', '0', 3, '%', 'F', '1', 3, '%', 'F', '2', 3, '%', 'F', '3',
    3, '%', 'F', '4', 3, '%', 'F', '5', 3, '%', 'F', '6', 3, '%', 'F', '7',
    3, '%', 'F', '8', 3, '%', 'F', '9', 3, '%', 'F', 'A', 3, '%', 'F', 'B',
    3, '%', 'F', 'C', 3, '%', 'F', 'D', 3, '%', 'F', 'E', 3, '%', 'F', 'F',
};

void StringUtility::UrlEncode(const std::string& src_str, std::string* dst_str)
{
    dst_str->clear();
    for (size_t i = 0; i < src_str.length() ; i++) {
        unsigned short offset = static_cast<unsigned short>(src_str[i]) * 4;
        dst_str->append((ENCODECHARS + offset + 1), ENCODECHARS[offset]);
    }
}

static const char HEX2DEC[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0 ,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

void StringUtility::UrlDecode(const std::string& src_str, std::string* dst_str)
{
    dst_str->clear();
    const unsigned char* src_begin = reinterpret_cast<const unsigned char*>(src_str.data());
    const unsigned char* src_end = src_begin + src_str.length();
    const unsigned char* src_last = src_end - 2;
    while (src_begin < src_last) {
        if ((*src_begin) == '%') {
            char dec1, dec2;
            if (-1 != (dec1 = HEX2DEC[*(src_begin + 1)])
                && -1 != (dec2 = HEX2DEC[*(src_begin + 2)])) {
                dst_str->append(1, (dec1 << 4) + dec2);
                src_begin += 3;
                continue;
            }
        } else if ((*src_begin) == '+') {
            dst_str->append(1, ' ');
            ++src_begin;
            continue;
        }
        dst_str->append(1, static_cast<char>(*src_begin));
        ++src_begin;
    }
    while (src_begin < src_end) {
        dst_str->append(1, static_cast<char>(*src_begin));
        ++src_begin;
    }
}

void StringUtility::ToUpper(std::string* str)
{
    std::transform(str->begin(), str->end(), str->begin(), ::toupper);
}

void StringUtility::ToLower(std::string* str)
{
    std::transform(str->begin(), str->end(), str->begin(), ::tolower);
}

bool StringUtility::StripSuffix(std::string* str, const std::string& suffix) {
    if (str->length() >= suffix.length()) {
        size_t suffix_pos = str->length() - suffix.length();
        if (str->compare(suffix_pos, std::string::npos, suffix) == 0) {
            str->resize(str->size() - suffix.size());
            return true;
        }
    }

    return false;
}

bool StringUtility::StripPrefix(std::string* str, const std::string& prefix) {
    if (str->length() >= prefix.length()) {
        if (str->substr(0, prefix.size()) == prefix) {
            *str = str->substr(prefix.size());
            return true;
        }
    }
    return false;
}

bool StringUtility::Hex2Bin(const char* hex_str, std::string* bin_str)
{
    if (nullptr == hex_str || nullptr == bin_str) {
        return false;
    }
    bin_str->clear();
    while (*hex_str != '\0') {
        if (hex_str[1] == '\0') {
            return false;
        }
        uint8 high = static_cast<uint8>(hex_str[0]);
        uint8 low = static_cast<uint8>(hex_str[1]);
#define ASCII2DEC(c) \
    if (c >= '0' && c <= '9') c -= '0'; \
        else if (c >= 'A' && c <= 'F') c -= ('A' - 10); \
        else if (c >= 'a' && c <= 'f') c -= ('a' - 10); \
        else return false
        ASCII2DEC(high);
        ASCII2DEC(low);
        bin_str->append(1, static_cast<char>((high << 4) + low));
        hex_str += 2;
    }
    return true;
}

bool StringUtility::Bin2Hex(const char* bin_str, std::string* hex_str)
{
    if (nullptr == bin_str || nullptr == hex_str) {
        return false;
    }
    hex_str->clear();
    while (*bin_str != '\0') {
        uint8 high = (static_cast<uint8>(*bin_str) >> 4);
        uint8 low = (static_cast<uint8>(*bin_str) & 0xF);
#define DEC2ASCII(c) \
    if (c <= 9) c += '0'; \
    else c += ('A' -  10)
        DEC2ASCII(high);
        DEC2ASCII(low);
        hex_str->append(1, static_cast<char>(high));
        hex_str->append(1, static_cast<char>(low));
        bin_str += 1;
    }
    return true;
}

void StringUtility::UTF8Printf(FILE* out, const char *str, ...)
{
	va_list ap;
	va_start(ap, str);
	VUTF8Printf(out, str, &ap);
	va_end(ap);
}

void StringUtility::VUTF8Printf(FILE* out, const char *str, va_list* ap)
{
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS 
	char temp_buf[32 * 1024] = {};
	wchar_t wtemp_buf[32 * 1024] = {};

	size_t temp_len = vsprintf_s(temp_buf, 32 * 1024, str, *ap);

	/*if (temp_len == size_t(-1))
	{
		temp_len = 32 * 1024 - 1;
	}

	size_t wtemp_len = 32 * 1024 - 1;

	UTF8ToWSTR(temp_buf, temp_len, wtemp_len, wtemp_len);
	CharToOemBuffW(&wtemp_buf[0], &temp_buf[0], uint32(wtemp_len + 1));*/
	fprintf(out, "%s", temp_buf);
#else
	vfprintf(out, str, *ap);
#endif
}

bool StringUtility::Utf8toWStr(const std::string & utf8str, std::wstring & wstr)
{
	wstr.clear();
	try
	{
		utf8::utf8to16(utf8str.c_str(), utf8str.c_str() + utf8str.size(), std::back_inserter(wstr));
	}
	catch (std::exception const&)
	{
		wstr.clear();
		return false;
	}

	return true;
}

bool StringUtility::IsBasicLatinCharacter(wchar_t wchar)
{
	if (wchar >= L'a' && wchar <= L'z')                      // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
		return true;
	if (wchar >= L'A' && wchar <= L'Z')                      // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
		return true;
	return false;
}

wchar_t StringUtility::wcharToUpper(wchar_t wchar)
{
	if (wchar >= L'a' && wchar <= L'z')                      // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
		return wchar_t(uint16(wchar) - 0x0020);
	if (wchar == 0x00DF)                                     // LATIN SMALL LETTER SHARP S
		return wchar_t(0x1E9E);
	if (wchar >= 0x00E0 && wchar <= 0x00F6)                  // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
		return wchar_t(uint16(wchar) - 0x0020);
	if (wchar >= 0x00F8 && wchar <= 0x00FE)                  // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
		return wchar_t(uint16(wchar) - 0x0020);
	if (wchar >= 0x0101 && wchar <= 0x012F)                  // LATIN SMALL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK (only %2=1)
	{
		if (wchar % 2 == 1)
			return wchar_t(uint16(wchar) - 0x0001);
	}
	if (wchar >= 0x0430 && wchar <= 0x044F)                  // CYRILLIC SMALL LETTER A - CYRILLIC SMALL LETTER YA
		return wchar_t(uint16(wchar) - 0x0020);
	if (wchar == 0x0451)                                     // CYRILLIC SMALL LETTER IO
		return wchar_t(0x0401);

	return wchar;
}

wchar_t StringUtility::wcharToUpperOnlyLatin(wchar_t wchar)
{
	return IsBasicLatinCharacter(wchar) ? wcharToUpper(wchar) : wchar;
}

bool StringUtility::WStrToUtf8(std::wstring const & wstr, std::string & utf8str)
{
	try
	{
		std::string utf8str2;
		utf8str2.resize(wstr.size() * 4);                     // allocate for most long case

		if (wstr.size())
		{
			char* oend = utf8::utf16to8(wstr.c_str(), wstr.c_str() + wstr.size(), &utf8str2[0]);
			utf8str2.resize(oend - (&utf8str2[0]));                // remove unused tail
		}
		utf8str = utf8str2;
	}
	catch (std::exception const&)
	{
		utf8str.clear();
		return false;
	}

	return true;
}

bool StringUtility::Utf8ToUpperOnlyLatin(std::string & utf8String)
{
	std::wstring wstr;
	if (!Utf8toWStr(utf8String, wstr))
		return false;

	std::transform(wstr.begin(), wstr.end(), wstr.begin(), wcharToUpperOnlyLatin);

	return WStrToUtf8(wstr, utf8String);
}

#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS 
std::wstring StringUtility::StringToWString(const std::string & str)
{
	std::wstring wstr;
	wstr.resize(str.size());
	OemToCharBuffW(&str[0], &wstr[0], uint32(str.size()));
	return wstr;
}

std::string StringUtility::WStringToString(const std::wstring & wstr)
{
	std::string str;
	str.resize(wstr.size());
	CharToOemBuffW(&wstr[0], &str[0], uint32(wstr.size()));
	return str;
}
#endif

bool StringUtility::ConsoleToUtf8(const std::string & conStr, std::string & utf8str)
{
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS 
	std::wstring wstr = StringToWString(conStr);
	//wstr.resize(conStr.size());
	//OemToCharBuffW(&conStr[0], &wstr[0], uint32(conStr.size()));

	return WStrToUtf8(wstr, utf8str);
#else
	// not implemented yet
	conStr = utf8str;
	return true;
#endif
}

bool StringUtility::Utf8ToConsole(const std::string& utf8str, std::string& conStr)
{
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS 
	std::wstring wstr;
	if (!Utf8toWStr(utf8str, wstr))
		return false;

	//conStr.resize(wstr.size());
	//CharToOemBuffW(&wstr[0], &conStr[0], uint32(wstr.size()));
	conStr = WStringToString(wstr);
#else
	// not implemented yet
	conStr = utf8str;
#endif
	return true;
}

#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS 
tm * StringUtility::localtime_r(time_t const * time, tm * result)
{
	localtime_s(result, time);
	return result;
}
#endif

Tokenizer::Tokenizer(const std::string & src, char const sep, uint32 vectorReserve, bool keepEmptyStrings)
{
	m_str = new char[src.length() + 1];
	memcpy(m_str, src.c_str(), src.length() + 1);

	if (vectorReserve)
		m_storage.reserve(vectorReserve);

	char* posold = m_str;
	char* posnew = m_str;

	for (;;)
	{
		if (*posnew == sep)
		{
			if (keepEmptyStrings || posold != posnew)
				m_storage.push_back(posold);

			posold = posnew + 1;
			*posnew = '\0';
		}
		else if (*posnew == '\0')
		{
			// Hack like, but the old code accepted these kind of broken strings,
			// so changing it would break other things
			if (posold != posnew)
				m_storage.push_back(posold);

			break;
		}

		++posnew;
	}
}

} // namespace pebble

