#pragma once

#include <string>

namespace gm
{
	struct EnvironmentMapData;

	class BinaryEnvironmentMapLoader
	{
	public:
		static bool Load(const std::wstring& filePath, EnvironmentMapData& outMapData);
	};
}
