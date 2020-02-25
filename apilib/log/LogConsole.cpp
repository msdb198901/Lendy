#include "LogConsole.h"
#include "StringUtility.h"
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
#include <Windows.h>
#endif
#include <sstream>

namespace LogComm
{
	using Util::StringUtility;

	LogConsole::LogConsole(std::string && extraArgs)
	{
		for (uint8 i = 0; i < NUM_ENABLED_LOG_LEVELS; ++i)
			m_colors[i] = ColorTypes(MaxColors - 1);

		if (!extraArgs.empty())
			InitColors(extraArgs);
	}
	void LogConsole::write(LogMessage const* message)
	{
		bool stdout_stream = !(message->level == LOG_LEVEL_ERROR || message->level == LOG_LEVEL_FATAL);

		if (m_colored)
		{
			uint8 index;
			switch (message->level)
			{
			case LOG_LEVEL_TRACE:
				index = 0;
				break;
			case LOG_LEVEL_DEBUG:
				index = 1;
				break;
			case LOG_LEVEL_INFO:
				index = 2;
				break;
			case LOG_LEVEL_WARN:
				index = 3;
				break;
			case LOG_LEVEL_FATAL:
				index = 4;
				break;
			case LOG_LEVEL_ERROR:
				index = 5;
				break;
			default:
				index = 0;
				break;
			}
			SetColor(stdout_stream, m_colors[index]);
			StringUtility::UTF8Printf(stdout_stream ? stdout : stderr, "%s%s\n", message->prefix.c_str(), message->text.c_str());
			ResetColor(stdout_stream);
		}
		else
		{
			StringUtility::UTF8Printf(stdout_stream ? stdout : stderr, "%s%s\n", message->prefix.c_str(), message->text.c_str());
		}
	}

	void LogConsole::InitColors(const std::string & str)
	{
		if (str.empty())
		{
			m_colored = false;
			return;
		}

		int color[NUM_ENABLED_LOG_LEVELS];

		std::istringstream ss(str);

		for (uint8 i = 0; i < NUM_ENABLED_LOG_LEVELS; ++i)
		{
			ss >> color[i];

			if (!ss)
				return;

			if (color[i] < 0 || color[i] >= MaxColors)
				return;
		}

		for (uint8 i = 0; i < NUM_ENABLED_LOG_LEVELS; ++i)
			m_colors[i] = ColorTypes(color[i]);

		m_colored = true;
	}
	void LogConsole::SetColor(bool stdout_stream, ColorTypes color)
	{
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
		static uint16 WinColorFG[MaxColors] =
		{
			0,															// BLACK
			FOREGROUND_RED,												// RED
			FOREGROUND_GREEN,											// GREEN
			FOREGROUND_RED | FOREGROUND_GREEN,							// BROWN
			FOREGROUND_BLUE,											// BLUE
			FOREGROUND_RED | FOREGROUND_BLUE,							// MAGENTA
			FOREGROUND_GREEN | FOREGROUND_BLUE,							// CYAN
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,		// WHITE
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,	// YELLOW	
			FOREGROUND_RED | FOREGROUND_INTENSITY,						// RED_BOLD
			FOREGROUND_GREEN | FOREGROUND_INTENSITY,					// GREEN_BOLD
			FOREGROUND_BLUE | FOREGROUND_INTENSITY,						// BLUE_BOLD
			FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,	// MAGENTA_BOLD
			FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,	// CYAN_BOLD
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY// WHITE_BOLD
		};
		HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
		SetConsoleTextAttribute(hConsole, WinColorFG[color]);
#else
		enum ANSITextAttr
		{
			TA_NORMAL = 0,
			TA_BOLD = 1,
			TA_BLINK = 5,
			TA_REVERSE = 7
		};

		enum ANSIFgTextAttr
		{
			FG_BLACK = 30,
			FG_RED,
			FG_GREEN,
			FG_BROWN,
			FG_BLUE,
			FG_MAGENTA,
			FG_CYAN,
			FG_WHITE,
			FG_YELLOW
		};

		enum ANSIBgTextAttr
		{
			BG_BLACK = 40,
			BG_RED,
			BG_GREEN,
			BG_BROWN,
			BG_BLUE,
			BG_MAGENTA,
			BG_CYAN,
			BG_WHITE
		};

		static uint8 UnixColorFG[MaxColors] =
		{
			FG_BLACK,                                          // BLACK
			FG_RED,                                            // RED
			FG_GREEN,                                          // GREEN
			FG_BROWN,                                          // BROWN
			FG_BLUE,                                           // BLUE
			FG_MAGENTA,                                        // MAGENTA
			FG_CYAN,                                           // CYAN
			FG_WHITE,                                          // WHITE
			FG_YELLOW,                                         // YELLOW
			FG_RED,                                            // LRED
			FG_GREEN,                                          // LGREEN
			FG_BLUE,                                           // LBLUE
			FG_MAGENTA,                                        // LMAGENTA
			FG_CYAN,                                           // LCYAN
			FG_WHITE                                           // LWHITE
		};

		fprintf((stdout_stream ? stdout : stderr), "\x1b[%d%sm", UnixColorFG[color], (color >= YELLOW && color < MaxColors ? ";1" : ""));
#endif
	}
	void LogConsole::ResetColor(bool stdout_stream)
	{
#if LENDY_PLATFORM == LENDY_PLATFORM_WINDOWS
		HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
		SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
#else
		fprintf((stdout_stream ? stdout : stderr), "\x1b[0m");
#endif
	}
}