#pragma once

#include "Component.h"
#include "LightTypes.h"

namespace gm
{
	struct LightRenderItem;
	class TransformComponent;

	class LightComponent : public Component
	{
	public:
		LightComponent() = default;
		~LightComponent() override;

		virtual LightType GetLightType() const = 0;

		void			SetColor(const Color& color) { _color = color; }
		const Color&	GetColor() const { return _color; }
		void			SetIntensity(float intensity);
		float			GetIntensity() const { return _intensity; }

		Vector3			GetPosition() const;
		Vector3			GetDirection() const;
		Vector3			GetRight() const;

	protected:
		void			OnInitialize() override;
		void			OnRender() override;
		virtual void	FillRenderItem(LightRenderItem& item) const {}

	private:
		TransformComponent*	_transform = nullptr;
		Color				_color = Colors::White;
		float				_intensity = 1.f;
	};
}
