#pragma once

#include "Entity.h"
#include "EngineCore.h"

namespace gm
{
	enum class ResourceType
	{
		Texture,
		Audio,
		Material,
		Mesh,
		Skeleton,
		SpriteAnimationClip,
		Shader,
		PipelineState,
		Sampler,

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
