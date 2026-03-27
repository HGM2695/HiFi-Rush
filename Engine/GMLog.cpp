#include "GMLog.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdarg>
#include <cstdio>

namespace gm
{
	void LogDebugOutput(const char* level, const char* format, ...)
	{
		char messageBuffer[768]{};
		va_list args{};
		va_start(args, format);
		std::vsnprintf(messageBuffer, sizeof(messageBuffer), format ? format : "", args);
		va_end(args);

		char outputBuffer[1024]{};
		std::snprintf(outputBuffer, sizeof(outputBuffer), "[GM_LOG][%s] %s\n", level ? level : "", messageBuffer);
		OutputDebugStringA(outputBuffer);
	}
}
