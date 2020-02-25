#ifndef TIMER_H
#define TIMER_H

#include "Define.h"
#include <chrono>

namespace DB
{
	inline std::chrono::steady_clock::time_point GetApplicationStartTime()
	{
		using namespace std::chrono;

		static const steady_clock::time_point ApplicationStartTime = steady_clock::now();

		return ApplicationStartTime;
	}

	inline uint32 getMSTime()
	{
		using namespace std::chrono;

		return uint32(duration_cast<milliseconds>(steady_clock::now() - GetApplicationStartTime()).count());
	}


	inline uint32 getMSTimeDiff(uint32 oldMSTime, uint32 newMSTime)
	{
		if (oldMSTime > newMSTime)
			return (0xFFFFFFFF - oldMSTime) + newMSTime;
		else
			return newMSTime - oldMSTime;
	}

	inline uint32 getMSTimeDiff(uint32 oldMSTime, std::chrono::steady_clock::time_point newTime)
	{
		using namespace std::chrono;

		uint32 newMSTime = uint32(duration_cast<milliseconds>(newTime - GetApplicationStartTime()).count());
		return getMSTimeDiff(oldMSTime, newMSTime);
	}

	inline uint32 GetMSTimeDiffToNow(uint32 oldMSTime)
	{
		return getMSTimeDiff(oldMSTime, getMSTime());
	}
}

#endif