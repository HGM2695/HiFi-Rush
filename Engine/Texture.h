#pragma once

#include "EngineCore.h"
#include "Resource.h"

namespace gm
{
	struct TextureDesc
	{
		std::wstring path;
	};

	class Texture : public Resource
	{
	public:
		virtual ~Texture() = default;

		static constexpr ResourceType Type = ResourceType::Texture;
		virtual	ResourceType	GetType() const override { return Type; }

		const std::wstring&		GetPath() const { return _path; }
		uint32                  GetWidth() const { return _width; }
		uint32                  GetHeight() const { return _height; }

	protected:
		Texture(const TextureDesc& desc, uint32 width, uint32 height) : _path(desc.path), _width(width), _height(height) {}

	private:
		std::wstring	_path;
		uint32			_width = 0;
		uint32			_height = 0;
	};
}
