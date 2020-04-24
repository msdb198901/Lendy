#include "INIReader.h"
#include "StringUtility.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <utility>

namespace Util
{
	INIReader * INIReader::GetInstance()
	{
		static INIReader m_pInstance;
		return &m_pInstance;
	}

	int INIReader::Parse(const std::string & filename)
	{
		FILE* file = nullptr;
			
		fopen_s(&file, filename.c_str(), "r");
		if (NULL == file) 
		{
			//_LOG_LAST_ERROR("open %s failed(%d:%s)", filename.c_str(), errno, strerror(errno));
			return -1;
		}

		//Clear();

		int ret = ParseFile(file);
		fclose(file);

		return ret;
	}

	void INIReader::Clear()
	{
		m_sections.clear();
		m_fields.clear();
	}

	std::string INIReader::Get(const std::string & section, const std::string & name, const std::string & default_value)
	{
		std::map<std::string, std::map<std::string, std::string> >::iterator it = m_fields.find(section);
		if (m_fields.end() == it) 
		{
			return default_value;
		}

		std::map<std::string, std::string>& fields_map = it->second;
		std::map<std::string, std::string>::iterator cit = fields_map.find(name);
		if (fields_map.end() == cit) 
		{
			return default_value;
		}
		return cit->second;
	}

	int INIReader::GetInt32(const std::string & section, const std::string & name, int default_value)
	{
		std::string value = Get(section, name, "");

		const char* begin = value.c_str();
		char* end = NULL;

		int n = strtol(begin, &end, 0);
		return end > begin ? n : default_value;
	}

	uint32 INIReader::GetUInt32(const std::string & section, const std::string & name, uint32 default_value)
	{
		std::string value = Get(section, name, "");
		const char* begin = value.c_str();
		char* end = NULL;

		int n = strtol(begin, &end, 0);
		if (end > begin && n >= 0) {
			return n;
		}
		return default_value;
	}

	int64 INIReader::GetInt64(const std::string & section, const std::string & name, int64 default_value)
	{
		std::string value = Get(section, name, "");
		const char* begin = value.c_str();
		char* end = NULL;

		int64 n = strtoll(begin, &end, 0);
		return end > begin ? n : default_value;
	}

	uint64 INIReader::GetUInt64(const std::string & section, const std::string & name, uint64 default_value)
	{
		std::string value = Get(section, name, "");
		const char* begin = value.c_str();
		char* end = NULL;

		int64 n = strtoll(begin, &end, 0);
		if (end > begin && n >= 0) {
			return n;
		}
		return default_value;
	}

	double INIReader::GetDouble(const std::string & section, const std::string & name, double default_value)
	{
		std::string value = Get(section, name, "");
		const char* begin = value.c_str();
		char* end = NULL;
		double n = strtod(begin, &end);
		return end > begin ? n : default_value;
	}

	bool INIReader::GetBool(const std::string & section, const std::string & name, bool default_value)
	{
		std::string value = Get(section, name, "");

		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		if (value == "true" || value == "yes" || value == "on" || value == "1") {
			return true;
		}
		else if (value == "false" || value == "no" || value == "off" || value == "0") {
			return false;
		}
		else {
			return default_value;
		}
	}

	const std::set<std::string>& INIReader::GetSections() const
	{
		return m_sections;
	}

	std::set<std::string> INIReader::GetFields(const std::string & section)
	{
		std::set<std::string> fields;

		std::map<std::string, std::map<std::string, std::string> >::iterator it = m_fields.find(section);
		if (m_fields.end() == it) {
			return fields;
		}

		std::map<std::string, std::string>& fields_map = it->second;
		std::map<std::string, std::string>::iterator cit = fields_map.begin();
		for (; cit != fields_map.end(); ++cit) {
			fields.insert(cit->first);
		}

		return fields;
	}

	int INIReader::ParseFile(FILE * file)
	{
		static const int MAX_BUFF_LEN = 2048;
		char buff[MAX_BUFF_LEN] = { 0 };

		int line_no = 0;
		std::string utf8bom;
		utf8bom.push_back((char)0xEF);
		utf8bom.push_back((char)0xBB);
		utf8bom.push_back((char)0xBF);
		std::map<std::string, std::string>* fields_map = NULL;
		while (fgets(buff, MAX_BUFF_LEN, file) != NULL) 
		{
			line_no++;
			std::string line(buff);

			// 0. 支持UTF-8 BOM
			if (1 == line_no && line.find_first_of(utf8bom) == 0) 
			{
				line.erase(0, 3);
			}

			// 1. 去掉注释
			for (size_t i = 0; i < line.length(); ++i)
			{
				if (';' == line[i] || '#' == line[i]) 
				{
					line.erase(i);
					break;
				}
			}

			// 2. 去掉首尾空格
			StringUtility::Trim(line);
			// 3. 去掉空行
			if (line.empty()) 
			{
				continue;
			}

			// section
			if (line[0] == '[' && line[line.length() - 1] == ']') 
			{
				std::string section(line.substr(1, line.length() - 2));
				StringUtility::Trim(section);
				if (section.empty()) 
				{
					return line_no;
				}
				m_sections.insert(section);
				fields_map = &(m_fields[section]);
				continue;
			}

			if (NULL == fields_map)
			{
				continue;
			}

			// fileds
			size_t pos = line.find('=');
			if (std::string::npos == pos) 
			{
				continue;
			}
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			StringUtility::Trim(key);
			StringUtility::Trim(value);
			if (key.empty() || value.empty())
			{
				continue;
			}

			(*fields_map)[key] = value;
		}

		return 0;
	}

}