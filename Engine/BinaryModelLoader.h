#pragma once

#include "ModelTypes.h"

#include <string>

namespace gm
{
	class BinaryModelLoader
	{
	public:
		ModelData Load(const std::wstring& filepath);
	};
}
