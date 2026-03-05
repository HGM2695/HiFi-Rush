#pragma once

#include <exception>

namespace gm
{
    void AssertPopupAndOptionalBreak(
        const char* tag,
        const char* exprText,
        const char* msg,
        const char* file,
        const char* func,
        int line);
}

// Debug, Release에서 ASSERT 매크로는 동일하게 사용하고, IMPL을 분리 함.
#ifdef _DEBUG
#define GM_ASSERT_IMPL(tag, exprText, msg, file, func, line) \
    ::gm::AssertPopupAndOptionalBreak((tag), (exprText), (msg), (file), (func), (line))
#else
#define GM_ASSERT_IMPL(tag, exprText, msg, file, func, line) ((void)0)
#endif // _DEBUG


#define GM_ASSERT(expr, msg)                                                        \
do                                                                                  \
{                                                                                   \
    if (!(expr))                                                                    \
    {                                                                               \
        GM_ASSERT_IMPL("GM_ASSERT", #expr, (msg), __FILE__, __FUNCTION__, __LINE__);\
    }                                                                               \
} while (0)

#define GM_ASSERT_RETURN(expr, msg)                                                 \
do                                                                                  \
{                                                                                   \
    if (!(expr))                                                                    \
    {                                                                               \
        GM_ASSERT_IMPL("GM_ASSERT_RETURN", #expr, (msg), __FILE__, __FUNCTION__, __LINE__); \
        return;                                                                     \
    }                                                                               \
} while (0)

#define GM_ASSERT_RETURN_VAL(expr, val, msg)                                        \
do                                                                                  \
{                                                                                   \
    if (!(expr))                                                                    \
    {                                                                               \
        GM_ASSERT_IMPL("GM_ASSERT_RETURN_VAL", #expr, (msg), __FILE__, __FUNCTION__, __LINE__); \
        return (val);                                                               \
    }                                                                               \
} while (0)

#define GM_ASSERT_TERMINATE(expr, msg)                                              \
do                                                                                  \
{                                                                                   \
    if (!(expr))                                                                    \
    {                                                                               \
        GM_ASSERT_IMPL("GM_ASSERT_TERMINATE", #expr, (msg), __FILE__, __FUNCTION__, __LINE__); \
        std::terminate();                                                           \
    }                                                                               \
} while (0)