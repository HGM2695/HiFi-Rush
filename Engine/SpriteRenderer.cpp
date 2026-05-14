#include "SpriteRenderer.h"
#include "GameObject.h"
#include "Transform.h"
#include "Texture.h"
#include "Camera.h"

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

	void SpriteRenderer::OnRender()
	{
		if (_texture == nullptr)
			return;
	}
}
