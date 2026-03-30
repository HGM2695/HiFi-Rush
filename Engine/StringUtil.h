#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <string>

namespace gm
{
	inline std::string WideToUtf8(const std::wstring& text)
	{
		if (text.empty())
			return {};

		const int requiredSize = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (requiredSize <= 0)
			return {};

		std::string result(static_cast<size_t>(requiredSize), '\0');
		WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, result.data(), requiredSize, nullptr, nullptr);

		if (result.empty() == false && result.back() == '\0')
			result.pop_back();

		return result;
	}

	inline std::wstring Utf8ToWide(const char* text)
	{
		if (text == nullptr || text[0] == '\0')
			return {};

		const int requiredSize = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
		if (requiredSize <= 0)
			return {};

		std::wstring result(static_cast<size_t>(requiredSize), L'\0');
		MultiByteToWideChar(CP_UTF8, 0, text, -1, result.data(), requiredSize);

		if (result.empty() == false && result.back() == L'\0')
			result.pop_back();

		return result;
	}
}
