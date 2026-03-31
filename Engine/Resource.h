#pragma once

#include "Entity.h"

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

		Count
	};

	class Resource : public Entity
	{
		friend class Resources;

	public:
		Resource() = default;
		virtual ~Resource() = default;

		virtual	ResourceType	GetType() const = 0;

	protected:
		virtual bool			LoadInternal(const std::wstring& path) = 0;

	private:
		bool					Load(const std::wstring& path, bool isPersistent = false) { return LoadInternal(path); }
	};
}