#include "Image.h"
#include "Resources.h"
#include "Texture.h"
#include "Application.h"

namespace gm
{
	Image::Image() = default;
	Image::Image(const std::wstring& textureName) { SetTextureByName(textureName); }
	Image::~Image() = default;

	void Image::SetTextureByName(const std::wstring& textureName)
	{
		_texture = APPLICATION.GetResources().Find<Texture>(textureName);
		GM_ASSERT(_texture, "등록되지 않은 텍스쳐 [%ls] 입니다.", textureName.c_str());
	}

	void Image::OnRender(const Vector2& absolutePosition)
	{
		if (_texture == nullptr)
			return;


	}
}
