#ifndef INI_READER_H
#define INI_READER_H

#include "Define.h"
#include <string>
#include <set>
#include <map>

namespace Util
{
	class LENDY_COMMON_API INIReader
	{
	public:
		static INIReader* GetInstance();

		int Parse(const std::string& filename);

		void Clear();

		std::string Get(const std::string& section, const std::string& name, const std::string& default_value);

		int GetInt32(const std::string& section, const std::string& name, int default_value);

		uint32 GetUInt32(const std::string& section, const std::string& name, uint32 default_value);

		int64 GetInt64(const std::string& section, const std::string& name, int64 default_value);

		uint64 GetUInt64(const std::string& section, const std::string& name, uint64 default_value);

		double GetDouble(const std::string& section, const std::string& name, double default_value);

		bool GetBool(const std::string& section, const std::string& name, bool default_value);

		const std::set<std::string>& GetSections() const;

		std::set<std::string> GetFields(const std::string& section);

		const char* GetLastError() const 
		{
			return m_last_error;
		}

	private:
		INIReader() {}

		~INIReader() {}

	private:
		int ParseFile(FILE* file);

	private:
		char m_last_error[256];
		EXPORT_BEGIN
		std::set<std::string> m_sections;
		std::map<std::string, std::map<std::string, std::string> > m_fields;
		EXPORT_END
	};

#define sConfigMgr INIReader::GetInstance()
}

#endif