#pragma once

#include "NavigationTypes.h"

#include <string>

namespace gm
{
	class BinaryNavigationMeshLoader
	{
	public:
		static bool Load(const std::wstring& filePath, _Out_ NavigationMeshData& outData);
	};
}
