#ifndef LOG_H
#define LOG_H

#include "StringFormat.h"
#include "LogMessage.h"
#include "LogFile.h"
#include "LogConsole.h"
#include <memory>

#ifdef LENDY_COMPILER_14
using std::make_unique;
#else
namespace std
{
	template<typename T, typename... Ts>
	std::unique_ptr<T> make_unique(Ts&&... params)
	{
		return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
	}
}
#endif

namespace LogComm
{
	class LENDY_COMMON_API Log
	{
	private:
		Log();
		~Log();
		DELETE_COPY_ASSIGN(Log);

	public:
		static Log* GetInstance();

		void Initialize(const char* filename);

		void LoadFromConfig(const char* filename);

		bool ShouldLog(LogLevel level) const;

		bool SetLogLevel(LogLevel level, AppenderType type, AppenderFlags flags);

		template<typename Format, typename... Args>
		inline void OutMessage(std::string const& filter, LogLevel const level, Format&& fmt, Args&&... args)
		{
			_OutMessage(filter, level, Util::StringFormat(std::forward<Format>(fmt), std::forward<Args>(args)...));
		}

		void _OutMessage(std::string const& filter, LogLevel level, std::string&& message);

		std::string GetLogDir() { return m_logsDir; }

		std::string GetLogsTimeStamp() { return m_logsTimeStamp; }

		std::string GetTimeStr();

	private:
		void write(std::unique_ptr<LogMessage>&& msg);

		static char const* GetLogLevelString(LogLevel level);

		LogLevel m_level;
		AppenderType m_type;
		AppenderFlags m_flags;
		EXPORT_BEGIN
		std::shared_ptr<LogConsole>	m_pLogConsole;
		std::shared_ptr<LogFile>	m_pLogFile;
		std::string	m_colors;
		std::string	m_files;
		std::string m_logsDir;
		std::string m_logsTimeStamp;
		EXPORT_END
	};

#define sLogMgr Log::GetInstance()

#define LOG_EXCEPTION_FREE(filterType__, level__, ...)					\
	{																	\
	try																	\
        {																\
            sLogMgr->OutMessage(filterType__, level__, __VA_ARGS__);	\
        }																\
        catch (std::exception& e)										\
        {																\
            sLogMgr->OutMessage("server", LOG_LEVEL_ERROR, "Wrong format occurred (%s) at %s:%u.", \
                e.what(), __FILE__, __LINE__);							\
        }																\
    }

#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS							
#define LOG_MESSAGE_BODY(filterType__, level__, ...)					\
        __pragma(warning(push))                                         \
        __pragma(warning(disable:4127))                                 \
        do {                                                            \
            if (sLogMgr->ShouldLog(level__))							\
                LOG_EXCEPTION_FREE(filterType__, level__, __VA_ARGS__); \
        } while (0)                                                     \
        __pragma(warning(pop))
#else
#define LOG_MESSAGE_BODY(filterType__, level__, ...)					\
        do {                                                            \
            if (sLogMgr->ShouldLog(level__))							\
                LOG_EXCEPTION_FREE(filterType__, level__, __VA_ARGS__); \
        } while (0)                                                     
#endif

#define LOG_TRACE(filterType__, ...) \
    LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_TRACE, __VA_ARGS__)

#define LOG_DEBUG(filterType__, ...) \
    LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_INFO(filterType__, ...)  \
    LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_INFO, __VA_ARGS__)

#define LOG_WARN(filterType__, ...)  \
    LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_WARN, __VA_ARGS__)

#define LOG_ERROR(filterType__, ...) \
    LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_ERROR, __VA_ARGS__)

#define LOG_FATAL(filterType__, ...) \
    LOG_MESSAGE_BODY(filterType__, LOG_LEVEL_FATAL, __VA_ARGS__)
}

#endif