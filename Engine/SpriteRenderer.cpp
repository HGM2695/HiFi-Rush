#include "SpriteRenderer.h"
#include "GameObject.h"
#include "Transform.h"
#include "GMAssert.h"
#include "Texture.h"
#include "Camera.h"
#include <Windows.h>
#include <gdiplus.h>

namespace gm
{
	SpriteRenderer::SpriteRenderer() = default;
	SpriteRenderer::~SpriteRenderer() = default;

	void SpriteRenderer::SetTexture(const std::shared_ptr<Texture>& texture)
	{
		GM_ASSERT_RETURN(texture, "texture가 nullptr 입니다.");
		_texture = texture;
	}

	void SpriteRenderer::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<Transform>();
	}

	void SpriteRenderer::OnRender(HDC hDC)
	{
		GM_ASSERT(_texture, "SpriteRenderer 클래스는 Texture Resource를 필요로 합니다.");

		math::Vector2 pos = _ownerTransform->GetPosition();
		pos = Camera::MainWorldToScreen(pos);

		Gdiplus::Graphics graphics(hDC);
		UINT width = _texture->GetWidth();
		UINT height = _texture->GetHeight();
		graphics.DrawImage(_texture->GetImage(), Gdiplus::Rect(static_cast<int>(pos._x - width * 0.5), static_cast<int>(pos._y - height * 0.5), _texture->GetWidth(), _texture->GetHeight()));
	}
}