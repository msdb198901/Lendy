#include "Log.h"
#include "StringFormat.h"
#include "StringUtility.h"
#include "LogConsole.h"
#include "LogFile.h"
#include "INIReader.h"
#include <sstream>
#include <chrono>

namespace LogComm
{
	using namespace Util;


	////////////////////////////////////
	Log::Log() : 
		m_level(LOG_LEVEL_DISABLED),
		m_type(APPENDER_NONE),
		m_flags(AFLAGS_NONE),
		m_pLogConsole(nullptr),
		m_pLogFile(nullptr)
	{
		m_logsTimeStamp = "_" + GetTimeStr();
	}

	Log::~Log()
	{
	}
	Log * Log::GetInstance()
	{
		static Log _instance;
		return &_instance;
	}
	void Log::Initialize(const char* filename)
	{
		LoadFromConfig(filename);
	}
	void Log::LoadFromConfig(const char* filename)
	{
		if (-1 == sConfigMgr->Parse(filename))
		{
			assert(nullptr);
			return;
		}
		m_level = static_cast<LogLevel>(sConfigMgr->GetInt32("Log", "LogLevel", 0));
		m_flags = static_cast<AppenderFlags>(sConfigMgr->GetInt32("Log", "LogFlags", 0));
		m_type = static_cast<AppenderType>(sConfigMgr->GetInt32("Log", "LogType", 0));
		m_logsDir = sConfigMgr->Get("Log", "LogDir", "");
		m_colors = sConfigMgr->Get("Log", "LogColor", "");
		m_files = sConfigMgr->Get("Log", "LogFile", "");

		if (!m_logsDir.empty())
		{
			if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
			{
				m_logsDir.push_back('/');
			}
		}

		m_pLogConsole = std::make_shared<LogConsole>(std::move(m_colors));
		m_pLogFile = std::make_shared<LogFile>(std::move(m_files));
	}
	bool Log::ShouldLog(LogLevel level) const
	{
		if (level < m_level)
		{
			return false;
		}
		return true;
	}
	bool Log::SetLogLevel(LogLevel level, AppenderType type, AppenderFlags flags)
	{
		m_level = level;
		m_flags = flags;
		m_type = type;
		return true;
	}
	void Log::_OutMessage(std::string const & filter, LogLevel level, std::string && message)
	{
		write(std::make_unique<LogMessage>(level, filter, std::move(message)));
	}
	void  Log::write(std::unique_ptr<LogMessage>&& message)
	{
		if (!m_level || m_level > message->level)
		{
			return;
		}

		std::ostringstream ss;

		if (m_flags & AFLAGS_PREFIX_TIMESTAMP)
		{
			ss << message->GetTimeStr() << ' ';
		}

		if (m_flags & AFLAGS_PREFIX_LOGLEVEL)
		{
			ss << Util::StringFormat("%-5s ", GetLogLevelString(message->level));
		}

		if (m_flags & AFLAGS_PREFIX_LOGFILTERTYPE)
		{
			ss << '[' << message->type << "] ";
		}

		message->prefix = ss.str();

		if (m_type | APPENDER_CONSOLE)
		{
			m_pLogConsole->write(message.get());
		}
		if (m_type | APPENDER_FILE)
		{
			m_pLogFile->write(message.get());
		}
	}
	char const*  Log::GetLogLevelString(LogLevel level)
	{
		switch (level)
		{
		case LOG_LEVEL_FATAL:
			return "FATAL";
		case LOG_LEVEL_ERROR:
			return "ERROR";
		case LOG_LEVEL_WARN:
			return "WARN";
		case LOG_LEVEL_INFO:
			return "INFO";
		case LOG_LEVEL_DEBUG:
			return "DEBUG";
		case LOG_LEVEL_TRACE:
			return "TRACE";
		default:
			return "DISABLED";
		}
	}

	std::string Log::GetTimeStr()
	{
		time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		std::tm aTm;
		Util::StringUtility::localtime_r(&tt, &aTm);

		return Util::StringFormat("%04d-%02d-%02d_%02d-%02d-%02d",
			aTm.tm_year + 1900, aTm.tm_mon + 1, aTm.tm_mday, aTm.tm_hour, aTm.tm_min, aTm.tm_sec);
	}
}