#ifndef LOG_CONSOLE_H
#define LOG_CONSOLE_H

#include "LogMessage.h"
#include <vector>

namespace LogComm
{
	enum ColorTypes
	{
		BLACK,
		RED,
		GREEN,
		BROWN,
		BLUE,
		MAGENTA,
		CYAN,
		GREY,
		YELLOW,
		LRED,
		LGREEN,
		LBLUE,
		LMAGENTA,
		LCYAN,
		WHITE
	};

	const uint8 MaxColors = uint8(WHITE) + 1;

	class LogConsole
	{
	public:
		LogConsole(std::string&& extraArgs);

		void write(LogMessage const* message);

	private:
		void InitColors(const std::string& init);
		void SetColor(bool stdout_stream, ColorTypes color);
		void ResetColor(bool stdout_stream);

		bool m_colored;
		ColorTypes m_colors[NUM_ENABLED_LOG_LEVELS];
	};
}

#endif