#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>

namespace gm
{
	// Boost hash_combine 방식 참고
	// https://www.boost.org/doc/libs/1_35_0/doc/html/boost/hash_combine.html
	inline void HashCombine(size_t& seed, size_t value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <typename T>
	inline void HashValue(size_t& seed, const T& value)
	{
		HashCombine(seed, std::hash<T>{}(value));
	}

	template <typename T>
	inline void HashEnum(size_t& seed, T value)
	{
		using UnderlyingType = std::underlying_type_t<T>;
		HashCombine(seed, std::hash<UnderlyingType>{}(static_cast<UnderlyingType>(value)));
	}
}
