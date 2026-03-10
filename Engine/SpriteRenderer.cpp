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
		_transform = GetOwner().GetComponent<Transform>();
	}

	void SpriteRenderer::OnRender(HDC hDC)
	{
		GM_ASSERT(_texture, "SpriteRenderer 클래스는 Texture Resource를 필요로 합니다.");

		math::Vector2 pos = _transform->GetPosition();
		pos = Camera::MainWorldToScreen(pos);

		Gdiplus::Graphics graphics(hDC);
		graphics.DrawImage(_texture->GetImage(), Gdiplus::Rect(static_cast<int>(pos._x), static_cast<int>(pos._y), _texture->GetWidth(), _texture->GetHeight()));
	}
}