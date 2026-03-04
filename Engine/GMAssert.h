#pragma once

#include <exception>

namespace gm
{
    void AssertPopupAndOptionalBreak(const char* tag, const char* exprText, const char* msg, const char* file, const char* func, int line);
}

#ifdef _DEBUG

#define GM_ASSERT(expr, msg)                                                        \
do                                                                                  \
{                                                                                   \
    if (!(expr))                                                                    \
    {                                                                               \
        ::gm::AssertPopupAndOptionalBreak("GM_ASSERT", #expr, (msg), __FILE__, __FUNCTION__, __LINE__); \
    }                                                                               \
} while (0)

#define GM_ASSERT_RETURN(expr, msg)                                                 \
do                                                                                  \
{                                                                                   \
    if (!(expr))                                                                    \
    {                                                                               \
        ::gm::AssertPopupAndOptionalBreak("GM_ASSERT_RETURN", #expr, (msg), __FILE__, __FUNCTION__, __LINE__); \
        return;                                                                     \
    }                                                                               \
} while (0)

#define GM_ASSERT_RETURN_VAL(expr, val, msg)                                        \
do                                                                                  \
{                                                                                   \
    if (!(expr))                                                                    \
    {                                                                               \
        ::gm::AssertPopupAndOptionalBreak("GM_ASSERT_RETURN_VAL", #expr, (msg), __FILE__, __FUNCTION__, __LINE__); \
        return (val);                                                               \
    }                                                                               \
} while (0)

#define GM_ASSERT_TERMINATE(expr, msg)                                              \
do                                                                                  \
{                                                                                   \
    if (!(expr))                                                                    \
    {                                                                               \
        ::gm::AssertPopupAndOptionalBreak("GM_ASSERT_TERMINATE", #expr, (msg), __FILE__, __FUNCTION__, __LINE__); \
        std::terminate();                                                           \
    }                                                                               \
} while (0)

#else

#define GM_ASSERT(expr, msg)                 ((void)0)
#define GM_ASSERT_RETURN(expr, msg)          ((void)0)
#define GM_ASSERT_RETURN_VAL(expr, val, msg) ((void)0)
#define GM_ASSERT_TERMINATE(expr, msg)       ((void)0)

#endif // _DEBUG