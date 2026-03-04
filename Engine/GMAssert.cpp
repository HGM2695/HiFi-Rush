#include "GMAssert.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <intrin.h>
#include <cwchar>

namespace gm
{
    static void Utf8ToWide(const char* src, wchar_t* dst, int dstCount)
    {
        if (!dst || dstCount <= 0) return;
        dst[0] = L'\0';

        if (!src || src[0] == '\0') return;

        int written = MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, dstCount);
        if (written == 0)
        {
            // 변환 실패 시 안전하게 비움
            dst[0] = L'\0';
        }
    }

    void AssertPopupAndOptionalBreak(
        const char* tag,
        const char* exprText,
        const char* msg,
        const char* file,
        const char* func,
        int line)
    {
        wchar_t wTag[128], wExpr[256], wMsg[512], wFile[512], wFunc[256];
        Utf8ToWide(tag, wTag, _countof(wTag));
        Utf8ToWide(exprText, wExpr, _countof(wExpr));
        Utf8ToWide(msg, wMsg, _countof(wMsg));
        Utf8ToWide(file, wFile, _countof(wFile));
        Utf8ToWide(func, wFunc, _countof(wFunc));

        wchar_t buffer[1024];
        _snwprintf_s(
            buffer, _countof(buffer), _TRUNCATE,
            L"[%s]\n\nExpression: %s\nMessage: %s\n\nFILE: %s\nFunc: %s\nLINE: %d",
            wTag, wExpr, wMsg, wFile, wFunc, line
        );

        int result = MessageBoxW(nullptr, buffer, L"GM_ASSERT", MB_OKCANCEL | MB_ICONERROR);

        if (result == IDOK && IsDebuggerPresent())
            __debugbreak();
    }
}