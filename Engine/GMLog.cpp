#include "GMLog.h"
#include "WindowsCore.h"

#include <cstdarg>
#include <cstdio>
#include <iterator>

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

		wchar_t wideOutputBuffer[1024]{};
		const int convertedLength = MultiByteToWideChar(CP_UTF8, 0, outputBuffer, -1, wideOutputBuffer, static_cast<int>(std::size(wideOutputBuffer)));
		if (convertedLength > 0)
			OutputDebugStringW(wideOutputBuffer);
		else
			OutputDebugStringA(outputBuffer);
	}
}
