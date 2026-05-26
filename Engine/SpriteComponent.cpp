#include "SpriteComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

namespace gm
{
	SpriteComponent::SpriteComponent() = default;
	SpriteComponent::~SpriteComponent() = default;

	void SpriteComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<TransformComponent>();
		GM_ASSERT_RETURN(_ownerTransform, "SpriteComponent 소유자의 Transform이 존재하지 않습니다.");

		_presenter.EnsureDefaultMaterial();
	}

	void SpriteComponent::OnRender()
	{
		_presenter.Submit(_ownerTransform->GetWorldMatrix());
	}

	void SpriteComponent::SetTexture(const std::shared_ptr<Texture>& texture, TextureSlot slot)
	{
		_presenter.SetTexture(texture, slot);
	}

	void SpriteComponent::SetSourceRect(const Rect& rect)
	{
		_presenter.SetSourceRect(rect);
	}

	void SpriteComponent::SetSourceRect(const SpriteFrame& frame)
	{
		_presenter.SetSourceRect(frame);
	}

	void SpriteComponent::DisableSourceRect()
	{
		_presenter.DisableSourceRect();
	}
}
