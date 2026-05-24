#include "AnimatedSpriteComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

namespace gm
{
	AnimatedSpriteComponent::AnimatedSpriteComponent() = default;
	AnimatedSpriteComponent::~AnimatedSpriteComponent() = default;

	void AnimatedSpriteComponent::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<TransformComponent>();
		GM_ASSERT_RETURN(_ownerTransform, "AnimatedSpriteComponent 소유자의 Transform이 존재하지 않습니다.");

		_presenter.EnsureDefaultMaterial();
	}

	void AnimatedSpriteComponent::OnTick(float deltaTime)
	{
		_animator.Tick(deltaTime, _presenter);
	}

	void AnimatedSpriteComponent::OnRender()
	{
		GM_ASSERT_RETURN(_ownerTransform, "OnInitialize()가 먼저 호출되어야 합니다.");
		_presenter.Submit(_ownerTransform->GetWorldMatrix());
	}

	void AnimatedSpriteComponent::SetTexture(const std::shared_ptr<Texture>& texture, MaterialSlot slot)
	{
		_presenter.SetTexture(texture, slot);
	}

	void AnimatedSpriteComponent::SetSourceRect(const Rect& rect)
	{
		_presenter.SetSourceRect(rect);
	}

	void AnimatedSpriteComponent::SetSourceRect(const SpriteFrame& frame)
	{
		_presenter.SetSourceRect(frame);
	}

	void AnimatedSpriteComponent::DisableSourceRect()
	{
		_presenter.DisableSourceRect();
	}
}
