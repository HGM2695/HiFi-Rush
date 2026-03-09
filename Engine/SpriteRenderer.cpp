#include "SpriteRenderer.h"
#include "GameObject.h"
#include "Transform.h"
#include "GMAssert.h"
#include "Texture.h"
#include <Windows.h>
#include <gdiplus.h>

namespace gm
{
	SpriteRenderer::SpriteRenderer() = default;
	SpriteRenderer::~SpriteRenderer() = default;

	void SpriteRenderer::OnInitialize()
	{
		_transform = GetOwner().GetComponent<Transform>();
	}

	void SpriteRenderer::OnRender(HDC hDC)
	{
		GM_ASSERT(_texture, "SpriteRenderer 클래스는 Texture Resource를 필요로 합니다.");

		const math::Vector2 pos = _transform->GetPosition();

		Gdiplus::Graphics graphics(hDC);
		graphics.DrawImage(_texture->GetImage(), Gdiplus::Rect(static_cast<int>(pos._X), static_cast<int>(pos._Y), _texture->GetWidth(), _texture->GetHeight()));
	}
}