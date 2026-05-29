#pragma once

#include "StringUtil.h"

#include <string>

namespace gm
{
	inline std::string GetFileNameWithoutExtension(const std::string& path)
	{
		const size_t slash = path.find_last_of("/\\");
		const size_t start = slash == std::string::npos ? 0 : slash + 1;
		const size_t dot = path.find_last_of('.');
		const size_t end = dot == std::string::npos || dot < start ? path.size() : dot;

		return path.substr(start, end - start);
	}

	inline std::wstring GetFileNameWithoutExtension(const std::wstring& path)
	{
		const size_t slash = path.find_last_of(L"/\\");
		const size_t start = slash == std::wstring::npos ? 0 : slash + 1;
		const size_t dot = path.find_last_of(L'.');
		const size_t end = dot == std::wstring::npos || dot < start ? path.size() : dot;

		return path.substr(start, end - start);
	}
}
