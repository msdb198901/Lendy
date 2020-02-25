#ifndef LOG_FILE_H
#define LOG_FILE_H

#include "LogComm.h"
#include "LogMessage.h"
#include <atomic>

namespace LogComm
{
	class LogFile
	{
	public:
		LogFile(std::string extraArgs);
		virtual ~LogFile();
		void write(LogMessage const* message);
		FILE* OpenFile(std::string const& filename, std::string const& mode, bool backup);

	private:
		void CloseFile();
		FILE* m_logfile;
		std::string m_fileName;
		std::string m_logDir;
		bool m_dynamicName;
		bool m_backup;
		uint64 m_maxFileSize;
		std::atomic<uint64> m_fileSize;
	};
}

#endif