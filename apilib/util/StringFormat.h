#ifndef STRING_FORMAT_H
#define STRING_FORMAT_H

#include <string>
#include "fmt/printf.h"

namespace Util
{
	/// Default TC string format function.
	template<typename Format, typename... Args>
	inline std::string StringFormat(Format&& fmt, Args&&... args)
	{
		try
		{
			return fmt::sprintf(std::forward<Format>(fmt), std::forward<Args>(args)...);
		}
		catch (const fmt::FormatError& formatError)
		{
			std::string error = "An error occurred formatting string \"" + std::string(fmt) + "\" : " + std::string(formatError.what());
			return error;
		}
	}

	/// Returns true if the given char pointer is null.
	inline bool IsFormatEmptyOrNull(char const* fmt)
	{
		return fmt == nullptr;
	}

	/// Returns true if the given std::string is empty.
	inline bool IsFormatEmptyOrNull(std::string const& fmt)
	{
		return fmt.empty();
	}
}

#endif