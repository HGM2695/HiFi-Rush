#pragma once

#include "Component.h"
#include "Vector2.h"

namespace gm
{
	class Transform;

	class Camera : public Component
	{
	public:
		static Camera*			GetMainCamera();
		static void				SetMainCamera(Camera* camera);
		static math::Vector2	MainWorldToScreen(const math::Vector2& worldPos);

		math::Vector2			WorldToScreen(const math::Vector2& worldPos) const;

	protected:
		virtual void			OnInitialize() override;
		virtual void			OnLateUpdate() override;

	private:
		inline static Camera* _mainCamera = nullptr;

		Transform* _transform = nullptr;
	};
}