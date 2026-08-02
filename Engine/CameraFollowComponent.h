#pragma once

#include "Component.h"
#include "MathUtil.h"
#include "WeakGameObjectPtr.h"

namespace gm
{
	class GameObject;
	class SocketComponent;
	class TransformComponent;

	class CameraFollowComponent : public Component
	{
	public:
		virtual TickGroup GetTickGroup() const override { return TickGroup::Camera; }

		void			SetTarget(const GameObject& target, const std::wstring& socketName);
		void			ClearTarget();

		void			SetDistance(float distance) { _distance = distance; }
		void			SetHeight(float height) { _height = height; }

		void			SetYaw(float yawRadians) { _yaw = yawRadians; }
		void			SetPitch(float pitchRadians);
		void			SetPitchRange(float minPitchRadians, float maxPitchRadians);
		void			SetMouseSensitivity(float radiansPerPixel) { _mouseSensitivity = radiansPerPixel; }
		void			SetMouseControlEnabled(bool enabled) { _mouseControlEnabled = enabled; }
		void			SetBottomDistanceLimit(float distance) { _bottomDistanceLimit = distance; _isLimitBottom = true; }
		void			ClearBottomDistanceLimit() { _isLimitBottom = false; }

		void			AddYaw(float deltaRadians) { _yaw += deltaRadians; }
		void			AddPitch(float deltaRadians) { SetPitch(_pitch + deltaRadians); }

		float			GetDistance() const { return _distance; }
		float			GetHeight() const { return _height; }
		float			GetYaw() const { return _yaw; }
		float			GetPitch() const { return _pitch; }
		float			GetMouseSensitivity() const { return _mouseSensitivity; }
		bool			IsMouseControlEnabled() const { return _mouseControlEnabled; }
		float			GetMinimumHeightFromTarget() const { return _bottomDistanceLimit; }
		bool			HasMinimumHeightLimit() const { return _isLimitBottom; }

	protected:
		virtual void	OnInitialize() override;
		virtual void	OnTick(float deltaTime) override;

	private:
		void			UpdateOrbitInput();
		Vector3			GetTargetPosition() const;
		Vector3			CalcCameraPosition(const Vector3& targetPosition) const;

	private:
		TransformComponent*		_ownerTransform = nullptr;
		WeakGameObjectPtr		_target;
		const SocketComponent*	_targetSocketComponent = nullptr;
		std::wstring			_socketName;

		float					_distance = 0.1f;
		float					_height = 0.f;
		float					_yaw = 0.f;
		float					_pitch = 0.f;
		float					_minPitch = Math::DegreesToRadians(-75.f);
		float					_maxPitch = Math::DegreesToRadians(89.f);
		float					_mouseSensitivity = Math::DegreesToRadians(0.1f);
		bool					_mouseControlEnabled = true;
		float					_bottomDistanceLimit = 0.f;
		bool					_isLimitBottom = false;
	};
}
