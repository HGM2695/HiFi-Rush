#pragma once

#include "EngineCore.h"
#include "Resource.h"
#include <memory>

namespace Gdiplus
{
	class Image;
}

namespace gm
{
	class Texture : public Resource
	{
	public:
		static constexpr ResourceType Type = ResourceType::Texture;
		virtual	ResourceType	GetType() const override { return Type; }
			
		uint32					GetWidth() const { return _width; }
		uint32					GetHeight() const { return _height; }
		Gdiplus::Image*			GetImage() const { return _image.get(); }

	protected:
		virtual bool			LoadInternal(const std::wstring& path) override;

	private:
		std::unique_ptr<Gdiplus::Image>	 _image{};
		uint32							 _width{};
		uint32							 _height{};
	};
}
