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
#include <unordered_set>

namespace gm
{
    namespace
    {
        struct AssertSiteKey
        {
            const char* file = nullptr;
            const char* exprText = nullptr;
            int line = 0;

            bool operator==(const AssertSiteKey& other) const
            {
                return file == other.file &&
                    exprText == other.exprText &&
                    line == other.line;
            }
        };

        struct AssertSiteKeyHasher
        {
            size_t operator()(const AssertSiteKey& key) const
            {
                const size_t fileHash = std::hash<const void*>{}(key.file);
                const size_t exprHash = std::hash<const void*>{}(key.exprText);
                const size_t lineHash = std::hash<int>{}(key.line);
                return fileHash ^ (exprHash << 1) ^ (lineHash << 2);
            }
        };

        std::unordered_set<AssertSiteKey, AssertSiteKeyHasher> g_ignoredAssertSites{};
    }

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
        const AssertSiteKey key{ file, exprText, line };
        if (g_ignoredAssertSites.find(key) != g_ignoredAssertSites.end())
            return;

        wchar_t wTag[128], wExpr[256], wMsg[512], wFile[512], wFunc[256];
        Utf8ToWide(tag, wTag, _countof(wTag));
        Utf8ToWide(exprText, wExpr, _countof(wExpr));
        Utf8ToWide(msg, wMsg, _countof(wMsg));
        Utf8ToWide(file, wFile, _countof(wFile));
        Utf8ToWide(func, wFunc, _countof(wFunc));

        wchar_t buffer[1024];
        _snwprintf_s(
            buffer, _countof(buffer), _TRUNCATE,
            L"[%s]\n\nExpression: %s\nMessage: %s\n\nFILE: %s\nFunc: %s\nLINE: %d\n\n"
            L"예: 중단점 이동\n아니오: 무시(1회)\n취소: 무시(계속)",
            wTag, wExpr, wMsg, wFile, wFunc, line
        );

        const int result = MessageBoxW(nullptr, buffer, L"GM_ASSERT", MB_YESNOCANCEL | MB_ICONERROR);

        if (result == IDYES && IsDebuggerPresent())
            __debugbreak();
        else if (result == IDCANCEL)
            g_ignoredAssertSites.insert(key);
    }
}
