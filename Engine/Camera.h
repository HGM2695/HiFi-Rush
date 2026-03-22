#pragma once

#include "Component.h"
#include "Vector2.h"

namespace gm
{
	class Transform;

	class Camera : public Component
	{
	public:
		Camera() = default;
		virtual ~Camera() override;

		static Camera*			GetMainCamera();
		static void				SetMainCamera(Camera* camera);
		static math::Vector2	MainWorldToScreen(const math::Vector2& worldPos);

		math::Vector2			WorldToScreen(const math::Vector2& worldPos) const;
		math::Vector2			GetCameraPosition() const { return _cameraPosition; }
		float					GetFollowSpeed() const { return _followSpeed; }
		
		void					SetDeadZoneWidth(float width) { _deadZoneWidth = width; }
		void					SetDeadZoneHeight(float height) { _deadZoneHeight = height; }
		void					SetDeadZone(float width, float height) { SetDeadZoneWidth(width); SetDeadZoneHeight(height); }
		void					SetFollowSpeed(float followSpeed) { _followSpeed = followSpeed; }

	protected:
		virtual void			OnInitialize() override;
		virtual void			OnLateUpdate() override;

	private:
		void					FollowOwner();

	private:
		inline static Camera*	_mainCamera = nullptr;

		Transform*				_ownerTransform = nullptr;
		math::Vector2			_cameraPosition{};

		// 카메라가 움직이지 않는 Dead Zone 영역 크기
		float					_deadZoneWidth{};
		float					_deadZoneHeight{};
		float					_followSpeed{ 5.f };
	};
}
