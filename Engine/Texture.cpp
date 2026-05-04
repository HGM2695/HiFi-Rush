#include "Texture.h"
#include <Windows.h>
#include <gdiplus.h>

namespace gm
{
	std::shared_ptr<Texture> Texture::Create(const TextureDesc& desc)
	{
		Gdiplus::Image* image = new Gdiplus::Image(desc.path.c_str());
		if (image->GetLastStatus() != Gdiplus::Ok)
		{
			delete image;
			GM_ASSERT_RETURN_VAL(false, nullptr, "Image load failed");
		}

		return std::shared_ptr<Texture>(new Texture(image));
	}

	Texture::Texture(Gdiplus::Image* image) : _image(image), _width(_image->GetWidth()), _height(_image->GetHeight()) {}
	Texture::~Texture() = default;
}