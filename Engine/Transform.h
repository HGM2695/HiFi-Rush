#pragma once
#include "Component.h"
#include "Vector2.h"

namespace gm
{
	class Transform : public Component
	{
	public:
		Transform();
		virtual ~Transform();

		virtual void	OnInitialize() override;
		virtual void	OnUpdate() override;
		virtual void	OnLateUpdate() override;
		virtual void	OnRender(HDC hDC) override;

		void					SetPosition(const math::Vector2& position) { _Position = position; }
		const math::Vector2&	GetPosition() { return _Position; }
		float					GetX() { return _Position._X; }
		float					GetY() { return _Position._Y; }

	private:
		math::Vector2 _Position;
	};
}