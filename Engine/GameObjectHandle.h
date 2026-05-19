#pragma once

#include "EngineCore.h"

namespace gm
{
	inline constexpr uint32 InvalidGameObjectIndex = -1;

	struct GameObjectHandle
	{
		uint32 index = InvalidGameObjectIndex;
		uint32 generation = 0;

		bool IsValid() const { return index != InvalidGameObjectIndex; }

		friend bool operator==(const GameObjectHandle& lhs, const GameObjectHandle& rhs)
		{
			return lhs.index == rhs.index && lhs.generation == rhs.generation;
		}

		friend bool operator!=(const GameObjectHandle& lhs, const GameObjectHandle& rhs)
		{
			return !(lhs == rhs);
		}
	};
}
