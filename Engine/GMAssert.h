#pragma once

#include <exception>

namespace gm
{
	void AssertPopupAndOptionalBreak(const char* tag, const char* exprText, const char* file, const char* func, int line, const char* format, ...);
}

#ifdef _DEBUG
#define GM_ASSERT_IMPL(tag, exprText, file, func, line, ...) \
	::gm::AssertPopupAndOptionalBreak((tag), (exprText), (file), (func), (line), __VA_ARGS__)
#else
#define GM_ASSERT_IMPL(tag, exprText, file, func, line, ...) ((void)0)
#endif

#define GM_ASSERT(expr, ...)                                                        \
do                                                                                  \
{                                                                                   \
	if (!(expr))                                                                    \
	{                                                                               \
		GM_ASSERT_IMPL("GM_ASSERT", #expr, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
	}                                                                               \
} while (0)

#define GM_ASSERT_RETURN(expr, ...)                                                 \
do                                                                                  \
{                                                                                   \
	if (!(expr))                                                                    \
	{                                                                               \
		GM_ASSERT_IMPL("GM_ASSERT_RETURN", #expr, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
		return;                                                                     \
	}                                                                               \
} while (0)

#define GM_ASSERT_RETURN_VAL(expr, val, ...)                                        \
do                                                                                  \
{                                                                                   \
	if (!(expr))                                                                    \
	{                                                                               \
		GM_ASSERT_IMPL("GM_ASSERT_RETURN_VAL", #expr, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
		return (val);                                                               \
	}                                                                               \
} while (0)

#define GM_ASSERT_TERMINATE(expr, ...)                                              \
do                                                                                  \
{                                                                                   \
	if (!(expr))                                                                    \
	{                                                                               \
		GM_ASSERT_IMPL("GM_ASSERT_TERMINATE", #expr, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
		std::terminate();                                                           \
	}                                                                               \
} while (0)
