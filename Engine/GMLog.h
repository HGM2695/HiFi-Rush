#pragma once

#include "BuildConfig.h"

namespace gm
{
	void LogDebugOutput(const char* level, const char* format, ...);
}

#if GM_ENABLE_DEBUG_TOOLS
#define GM_LOG_IMPL(level, ...) ::gm::LogDebugOutput((level), __VA_ARGS__)
#else
#define GM_LOG_IMPL(level, ...) ((void)0)
#endif

#define GM_LOG(...) GM_LOG_IMPL("INFO", __VA_ARGS__)
#define GM_LOG_WARNING(...) GM_LOG_IMPL("WARN", __VA_ARGS__)
#define GM_LOG_ERROR(...) GM_LOG_IMPL("ERROR", __VA_ARGS__)
