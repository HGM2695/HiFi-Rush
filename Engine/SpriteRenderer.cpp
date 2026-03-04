#include "SpriteRenderer.h"
#include "GameObject.h"
#include "Transform.h"
#include <Windows.h>

namespace gm
{
	SpriteRenderer::SpriteRenderer() = default;
	SpriteRenderer::~SpriteRenderer() = default;

	void SpriteRenderer::OnInitialize()
	{
		_Transform = GetOwner().GetComponent<Transform>();
		assert(_Transform && "SpriteRenderer 클래스는 Transform 컴포넌트를 필요로합니다.");
	}

	void SpriteRenderer::OnUpdate()
	{
	}

	void SpriteRenderer::OnLateUpdate()
	{
	}

	void SpriteRenderer::OnRender(HDC hDC)
	{
		Rectangle(hDC, _Transform->GetX(), _Transform->GetY(), 100 + _Transform->GetX(), 100 + _Transform->GetY());
	}
}