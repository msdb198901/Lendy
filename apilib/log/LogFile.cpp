#include "Log.h"
#include "LogFile.h"
#include "StringUtility.h"
#include <algorithm>
#include <ctime>

namespace LogComm
{
	using namespace Comm;
	LogFile::LogFile(std::string extraArgs) : 
		m_logfile(nullptr), 
		m_logDir(sLogMgr->GetLogDir()), 
		m_maxFileSize(0), 
		m_fileSize(0)
	{

		Util::Tokenizer tokens(extraArgs, '|');
		// name|mode|flag|size|

		uint8 i = 0;
		uint8 len = tokens.size();
		if (i < len)
		{
			m_fileName = tokens[i++];
		}

		std::string mode = "a";
		if (i < len)
		{
			mode = tokens[i++];
		}

		int flags = AFLAGS_NONE;
		if (i < len)
		{
			flags = atoi(tokens[i++]);
		}

		if (flags & AFLAGS_USE_TIMESTAMP)
		{
			size_t dot_pos = m_fileName.find_last_of('.');
			if (dot_pos != std::string::npos)
			{
				m_fileName.insert(dot_pos, sLogMgr->GetLogsTimeStamp());
			}
			else
			{
				m_fileName += sLogMgr->GetLogsTimeStamp();
			}
		}
		
		if (i < len)
		{
			m_maxFileSize = atoi(tokens[i++]);
		}

		m_dynamicName = std::string::npos != m_fileName.find("%s");
		m_backup = (flags & AFLAGS_MAKE_FILE_BACKUP) != 0;

		if (!m_dynamicName)
		{
			m_logfile = OpenFile(m_fileName, mode, !strcmp(mode.c_str(), "w") && m_backup);
		}
	}

	LogFile::~LogFile()
	{
		CloseFile();
	}

	void LogFile::write(LogMessage const* message)
	{
		bool exceedMaxSize = m_maxFileSize > 0 && (m_fileSize.load() + message->Size()) > m_maxFileSize;

		if (m_dynamicName)
		{
			char namebuf[FILE_PATH_MAX] = {};
			snprintf(namebuf, FILE_PATH_MAX, m_fileName.c_str(), message->param1.c_str());

			FILE* file = OpenFile(namebuf, "a", m_backup || exceedMaxSize);
			if (!file)
			{
				return;
			}

			fprintf(file, "%s%s\n", message->prefix.c_str(), message->text.c_str());
			fflush(file);
			m_fileSize += uint64(message->Size());
			fclose(file);
			return;
		}
		else if (exceedMaxSize)
		{
			m_logfile = OpenFile(m_fileName, "w", true);
		}

		if (!m_logfile)
		{
			return;
		}

		fprintf(m_logfile, "%s%s\n", message->prefix.c_str(), message->text.c_str());
		fflush(m_logfile);
		m_fileSize += uint64(message->Size());
	}

	FILE* LogFile::OpenFile(std::string const& filename, std::string const& mode, bool backup)
	{
		std::string fullName(m_logDir + filename);
		if (backup)
		{
			CloseFile();
			std::string newName(fullName);
			newName.push_back('.');
			newName.append(sLogMgr->GetTimeStr());
			std::replace(newName.begin(), newName.end(), ':', '-');
			rename(fullName.c_str(), newName.c_str()); 
		}

		FILE* ret = nullptr;
		fopen_s(&ret, fullName.c_str(), mode.c_str());
		if (ret)
		{
			m_fileSize = ftell(ret);
			return ret;
		}
		return nullptr;
	}

	void LogFile::CloseFile()
	{
		if (m_logfile)
		{
			fclose(m_logfile);
			m_logfile = nullptr;
		}
	}
}