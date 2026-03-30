#include "GMAssert.h"
#include "StringUtil.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdarg>
#include <cwchar>
#include <intrin.h>
#include <cstdio>
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
				return file == other.file && exprText == other.exprText && line == other.line;
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

	void AssertPopupAndOptionalBreak(const char* tag, const char* exprText, const char* file, const char* func, int line, const char* format, ...)
	{
		const AssertSiteKey key{ file, exprText, line };
		if (g_ignoredAssertSites.find(key) != g_ignoredAssertSites.end())
			return;

		char messageBuffer[768]{};
		va_list args{};
		va_start(args, format);
		std::vsnprintf(messageBuffer, sizeof(messageBuffer), format ? format : "", args);
		va_end(args);

		const std::wstring wTag = Utf8ToWide(tag);
		const std::wstring wExpr = Utf8ToWide(exprText);
		const std::wstring wMsg = Utf8ToWide(messageBuffer);
		const std::wstring wFile = Utf8ToWide(file);
		const std::wstring wFunc = Utf8ToWide(func);

		wchar_t buffer[1024]{};
		_snwprintf_s(
			buffer, _countof(buffer), _TRUNCATE,
			L"[%s]\n\nExpression: %s\nMessage: %s\n\nFILE: %s\nFunc: %s\nLINE: %d\n\n"
			L"예: 중단점 이동\n아니오: 무시(1회)\n취소: 무시(계속)",
			wTag.c_str(), wExpr.c_str(), wMsg.c_str(), wFile.c_str(), wFunc.c_str(), line);

		const int result = MessageBoxW(nullptr, buffer, L"GM_ASSERT", MB_YESNOCANCEL | MB_ICONERROR);

		if (result == IDYES && IsDebuggerPresent())
			__debugbreak();
		else if (result == IDCANCEL)
			g_ignoredAssertSites.insert(key);
	}
}
