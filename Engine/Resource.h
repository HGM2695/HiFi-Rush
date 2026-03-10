#pragma once

#include "Entity.h"

namespace gm
{
	enum class ResourceType
	{
		Texture,
		Material,
		Mesh,
		Skeleton,
		AnimationClip,
		AudioClip,

		Count
	};

	class Resources;

	class Resource : public Entity
	{
		friend class Resources;

	public:
		Resource() = default;
		virtual ~Resource() = default;

		virtual	ResourceType	GetType() const = 0;

		const std::wstring&		GetPath() const { return _path; }

	protected:
		virtual bool			LoadInternal(const std::wstring& path) = 0;

	private:
		bool					Load(const std::wstring& path) { return LoadInternal(path); }
		void					SetPath(const std::wstring& path) { _path = path; }

	private:
		std::wstring _path;
	};
}