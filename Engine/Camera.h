#pragma once

#include "Component.h"

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
		static Vector2			MainWorldToScreen(const Vector2& worldPos);

		Vector2					WorldToScreen(const Vector2& worldPos) const;
		Vector2					GetCameraPosition() const { return _cameraPosition; }
		float					GetFollowSpeed() const { return _followSpeed; }
		
		void					SetDeadZoneWidth(float width) { _deadZoneWidth = width; }
		void					SetDeadZoneHeight(float height) { _deadZoneHeight = height; }
		void					SetDeadZone(float width, float height) { SetDeadZoneWidth(width); SetDeadZoneHeight(height); }
		void					SetFollowSpeed(float followSpeed) { _followSpeed = followSpeed; }
		virtual TickGroup		GetTickGroup() const override { return TickGroup::Camera; }

	protected:
		virtual void			OnInitialize() override;
		virtual void			OnTick(float deltaTime) override;

	private:
		void					FollowOwner(float deltaTime);

	private:
		inline static Camera*	_mainCamera = nullptr;

		Transform*				_ownerTransform = nullptr;
		Vector2					_cameraPosition{};

		// 카메라가 움직이지 않는 Dead Zone 영역 크기
		float					_deadZoneWidth{};
		float					_deadZoneHeight{};
		float					_followSpeed{ 5.f };
	};
}
