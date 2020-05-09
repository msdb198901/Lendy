#include "LogMessage.h"
#include "StringUtility.h"
#include <ctime>

namespace LogComm
{
	LogMessage::LogMessage(LogLevel _level, std::string const & _type, std::string && _text)
		: level(_level), type(_type), text(std::forward<std::string>(_text)), mtime(time(nullptr))
	{
	}

	LogMessage::LogMessage(LogLevel _level, std::string const & _type, std::string && _text, std::string && _param1)
		: level(_level), type(_type), text(std::forward<std::string>(_text)), param1(std::forward<std::string>(_param1)), mtime(time(nullptr))
	{
	}

	std::string LogMessage::GetTimeStr(time_t time)
	{
		tm _tm;
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
		Util::StringUtility::localtime_r(&time, &_tm);
#else
		localtime_r(&time, &_tm);
#endif
		char buf[20];
		snprintf(buf, 20, "%04d-%02d-%02d_%02d:%02d:%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
		return std::string(buf);
	}
	std::string LogMessage::GetTimeStr() const
	{
		return GetTimeStr(mtime);
	}
	uint32 LogMessage::Size() const
	{
		return static_cast<uint32>(prefix.size() + text.size());
	}
} 