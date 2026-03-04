#pragma once

#include "Component.h"

namespace gm
{
	class Transform;

	class SpriteRenderer : public Component
	{
	public:
		SpriteRenderer();
		virtual ~SpriteRenderer();

		virtual void	OnInitialize() override;
		virtual void	OnUpdate() override;
		virtual void	OnLateUpdate() override;
		virtual void	OnRender(HDC hDC) override;

	private:
		Transform* _Transform{};
	};
}