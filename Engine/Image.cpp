#include "Image.h"
#include "Resources.h"
#include "Texture.h"
#include "Application.h"
#include <Windows.h>
#include <gdiplus.h>

namespace gm
{
	Image::Image() = default;
	Image::Image(const std::wstring& textureName)
	{
		SetTextureByName(textureName);
	}
	Image::~Image() = default;

	void Image::SetTextureByName(const std::wstring& textureName)
	{
		_texture = APPLICATION.GetResources().Find<Texture>(textureName);
		GM_ASSERT(_texture, "등록되지 않은 텍스쳐 [%ls] 입니다.", textureName.c_str());
	}

	void Image::OnRender(HDC hDC, const math::Vector2& absolutePosition)
	{
		if (_texture == nullptr)
			return;

		Gdiplus::Graphics graphics(hDC);
		const math::Vector2 widgetSize = GetSize();
		const int drawWidth = widgetSize._x > 0.f ? static_cast<int>(widgetSize._x) : static_cast<int>(_texture->GetWidth());
		const int drawHeight = widgetSize._y > 0.f ? static_cast<int>(widgetSize._y) : static_cast<int>(_texture->GetHeight());

		graphics.DrawImage(_texture->GetImage(),
			Gdiplus::Rect(
				static_cast<int>(absolutePosition._x),
				static_cast<int>(absolutePosition._y),
				drawWidth, drawHeight));
	}
}
