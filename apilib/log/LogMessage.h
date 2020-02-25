#ifndef LOG_MESSAGE_H
#define LOG_MESSAGE_H

#include "Define.h"
#include "LogComm.h"
#include <string>

namespace LogComm
{
	struct LENDY_COMMON_API LogMessage
	{
		LogMessage(LogLevel _level, std::string const& _type, std::string&& _text);
		LogMessage(LogLevel _level, std::string const& _type, std::string&& _text, std::string&& _param1);

		static std::string GetTimeStr(time_t time);
		std::string GetTimeStr() const;

		LogLevel const level;

		EXPORT_BEGIN
		std::string const type;
		std::string const text;
		std::string prefix;
		std::string param1;
		EXPORT_END
		time_t mtime;

		uint32 Size() const;

		DELETE_COPY_ASSIGN(LogMessage);
	};
}

#endif