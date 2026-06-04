#pragma once

#include "Component.h"
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
		void			SetPitch(float pitchRadians) { _pitch = pitchRadians; }

		void			AddYaw(float deltaRadians) { _yaw += deltaRadians; }
		void			AddPitch(float deltaRadians) { _pitch += deltaRadians; }

		float			GetDistance() const { return _distance; }
		float			GetHeight() const { return _height; }
		float			GetYaw() const { return _yaw; }
		float			GetPitch() const { return _pitch; }

	protected:
		virtual void	OnInitialize() override;
		virtual void	OnTick(float deltaTime) override;

	private:
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
	};
}
