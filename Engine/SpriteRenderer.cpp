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

	void SpriteRenderer::ResetSourceRect()
	{
		_useSourceRect = false;
	}

	void SpriteRenderer::SetSourceRect(const SpriteFrame& frame)
	{
		_sourceFrame = frame;
		_useSourceRect = true;
	}

	void SpriteRenderer::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<Transform>();
	}

	void SpriteRenderer::OnRender(HDC hDC)
	{
		if (_texture == nullptr)
			return;

		math::Vector2 pos = _ownerTransform->GetPosition();
		pos = Camera::MainWorldToScreen(pos);

		Gdiplus::Graphics graphics(hDC);
		int drawWidth = static_cast<int>(_texture->GetWidth());
		int drawHeight = static_cast<int>(_texture->GetHeight());

		if (_useSourceRect)
		{
			drawWidth = _sourceFrame.width;
			drawHeight = _sourceFrame.height;
			graphics.DrawImage(
				_texture->GetImage(), Gdiplus::Rect(static_cast<int>(pos._x - drawWidth * 0.5f), static_cast<int>(pos._y - drawHeight * 0.5f), drawWidth, drawHeight),
				_sourceFrame.left, _sourceFrame.top, _sourceFrame.width, _sourceFrame.height, Gdiplus::UnitPixel
			);
			return;
		}

		graphics.DrawImage(_texture->GetImage(),
			Gdiplus::Rect(static_cast<int>(pos._x - drawWidth * 0.5f), static_cast<int>(pos._y - drawHeight * 0.5f), drawWidth, drawHeight));
	}
}
