#pragma once

#include "Types.h"

#include <istream>
#include <string>
#include <type_traits>
#include <vector>

namespace gm
{
	template <typename T>
	bool ReadBinary(std::istream& stream, T& out)
	{
		stream.read(reinterpret_cast<char*>(&out), sizeof(T));
		return stream.good();
	}

	template <typename T>
	bool ReadBinaryVector(std::istream& stream, std::vector<T>& out, uint32 count)
	{
		out.resize(count);
		if (count == 0)
			return true;

		stream.read(reinterpret_cast<char*>(out.data()), sizeof(T) * count);
		return stream.good();
	}

	inline bool ReadBinaryString(std::istream& stream, std::string& out)
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
}
