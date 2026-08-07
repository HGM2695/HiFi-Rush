#pragma once

#include "Entity.h"
#include "EngineCore.h"

namespace gm
{
	enum class ResourceType
	{
		Texture,
		Audio,
		Mesh,
		StaticMesh,
		SkeletalMesh,
		Skeleton,
		SpriteAnimationClip,
		SkeletalAnimationClip,
		Shader,
		NavigationMesh,
		Map,

		Count
	};

	class Resource : public Entity
	{
		friend class Resources;

	public:
		Resource() = default;
		virtual ~Resource() = default;

		virtual	ResourceType	GetType() const = 0;
	};
}
