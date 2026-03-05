#pragma once

#include "Entity.h"

namespace gm
{
	enum class ResourceType
	{
		AudioClip,
		Prefab,
		Texture,

		Count
	};

	class Resource : public Entity
	{
	public:
		Resource(ResourceType type);
		virtual ~Resource();

		virtual bool			Load(const std::wstring& path) = 0;
		ResourceType			GetType() { return _type; }
		const std::wstring&		GetPath() { return _path; }
		void					SetPath(const std::wstring& path) { _path = path; }

	private:
		ResourceType _type;
		std::wstring _path;
	};
}


