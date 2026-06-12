#pragma once

#include "Types.h"
#include "StringUtil.h"

#include <array>
#include <istream>
#include <string>
#include <type_traits>
#include <vector>

namespace gm
{
	template <typename T>
	bool ReadBinary(std::istream& stream, _Out_ T& out)
	{
		stream.read(reinterpret_cast<char*>(&out), sizeof(T));
		return stream.good();
	}

	template <typename T>
	bool ReadBinaryVector(std::istream& stream, _Out_ std::vector<T>& out, uint32 count)
	{
		out.resize(count);
		if (count == 0)
			return true;

		stream.read(reinterpret_cast<char*>(out.data()), sizeof(T) * count);
		return stream.good();
	}

	template <typename T, size_t Size>
	bool ReadBinaryArray(std::istream& stream, _Out_ std::array<T, Size>& out)
	{
		stream.read(reinterpret_cast<char*>(out.data()), sizeof(T) * Size);
		return stream.good();
	}

	inline bool ReadBinaryString(std::istream& stream, _Out_ std::string& out)
	{
		uint32 length = 0;
		if (ReadBinary(stream, length) == false)
			return false;

		out.resize(length);
		if (length == 0)
			return true;

		stream.read(out.data(), length);
		return stream.good();
	}

	inline bool ReadBinaryWideString(std::istream& inputStream, _Out_ std::wstring& out)
	{
		std::string text;
		if (ReadBinaryString(inputStream, text) == false)
			return false;

		out = Utf8ToWide(text.c_str());
		return true;
	}
}
