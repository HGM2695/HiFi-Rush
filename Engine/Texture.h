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
		Texture();
		virtual ~Texture();

		virtual bool			Load(const std::wstring& path) override;
			
		uint32_t				GetWidth() { return _width; }
		uint32_t				GetHeight() { return _height; }
		Gdiplus::Image*			GetImage() { return _image.get(); }

	private:
		std::unique_ptr<Gdiplus::Image>	 _image{};
		uint32_t						 _width{};
		uint32_t						 _height{};
	};
}


