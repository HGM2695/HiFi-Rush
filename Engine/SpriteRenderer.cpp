#include "SpriteRenderer.h"
#include "GameObject.h"
#include "Transform.h"
#include <Windows.h>
#include <gdiplus.h>
#include "../Engine/GMAssert.h"

namespace gm
{
	SpriteRenderer::SpriteRenderer() = default;
	SpriteRenderer::~SpriteRenderer() = default;

	void SpriteRenderer::OnInitialize()
	{
		SetName(L"Sprite Renderer");
		_Transform = GetOwner().GetComponent<Transform>();
		GM_ASSERT(_Transform, "SpriteRenderer 클래스는 Transform 컴포넌트를 필요로 합니다.");
	}

	void SpriteRenderer::OnUpdate()
	{
	}

	void SpriteRenderer::OnLateUpdate()
	{
	}

	void SpriteRenderer::OnRender(HDC hDC)
	{
		const math::Vector2 pos = _Transform->GetPosition();

		Gdiplus::Graphics graphics(hDC);
		graphics.DrawImage(_Image.get(), Gdiplus::Rect(static_cast<int>(pos._X), static_cast<int>(pos._Y), _Width, _Height));
	}

	void SpriteRenderer::ImageLoad(const std::wstring& path)
	{
		_Image = std::make_unique<Gdiplus::Image>(path.c_str());
		GM_ASSERT_RETURN(_Image, "Image 생성에 실패했습니다.");

		_Width = _Image->GetWidth();
		_Height = _Image->GetHeight();
	}
}