#pragma once

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
			
		uint32_t				GetWidth() const { return _width; }
		uint32_t				GetHeight() const { return _height; }
		Gdiplus::Image*			GetImage() const { return _image.get(); }

	protected:
		virtual bool			LoadInternal(const std::wstring& path) override;

	private:
		std::unique_ptr<Gdiplus::Image>	 _image{};
		uint32_t						 _width{};
		uint32_t						 _height{};
	};
}


