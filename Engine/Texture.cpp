#include "Texture.h"
#include <Windows.h>
#include <gdiplus.h>

namespace gm
{
	bool Texture::Load(const std::wstring& path)
	{
		_image = std::make_unique<Gdiplus::Image>(path.c_str());
		GM_ASSERT_RETURN_VAL(_image->GetLastStatus() == Gdiplus::Ok, false, "Image load failed");

		_width = _image->GetWidth();
		_height = _image->GetHeight();

		return true;
	}
}