#pragma once

#include "EngineCore.h"
#include "Resource.h"

namespace Gdiplus
{
	class Image;
}

namespace gm
{
	struct TextureDesc
	{
		std::wstring path;
	};

	class Texture : public Resource
	{
	public:
		static std::shared_ptr<Texture> Create(const TextureDesc& desc);
		~Texture();

		static constexpr ResourceType Type = ResourceType::Texture;
		virtual	ResourceType	GetType() const override { return Type; }
			
		uint32					GetWidth() const { return _width; }
		uint32					GetHeight() const { return _height; }
		Gdiplus::Image*			GetImage() const { return _image.get(); }

	private:
		Texture(Gdiplus::Image* image);

		std::unique_ptr<Gdiplus::Image>	 _image{};
		uint32							 _width{};
		uint32							 _height{};
	};
}
