#include "EffectSpriteComponent.h"

namespace gm
{
	EffectSpriteComponent::EffectSpriteComponent()
	{
		GetPresenter().EnableEffectRendering();
	}

	EffectSpriteComponent::~EffectSpriteComponent() = default;
}
