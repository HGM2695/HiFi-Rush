#pragma once

#include "Component.h"
#include "CameraViewInfo.h"

namespace gm
{
	class TransformComponent;

	enum class CameraProjectionMode
	{
		Orthographic,
		Perspective,
	};

	class CameraComponent : public Component
	{
	public:
		CameraComponent() = default;
		virtual ~CameraComponent() override;
		virtual TickGroup		GetTickGroup() const override { return TickGroup::Camera; }

		CameraViewInfo			GetViewInfo() const;
		Vector3					GetForwardDirection() const;
		Vector3					GetRightDirection() const;
		Vector3					GetUpDirection() const;

		void					SetOrthographic(float width, float height, float nearZ = 0.f, float farZ = 1000.f);
		void					SetPerspective(float fovYRadians, float aspectRatio, float nearZ = 0.1f, float farZ = 1000.f);

		CameraProjectionMode	GetProjectionMode() const { return _projectionMode; }
		float					GetFovYRadians() const { return _fovYRadians; }
		float					GetAspectRatio() const { return _aspectRatio; }
		float					GetNearZ() const { return _nearZ; }
		float					GetFarZ() const { return _farZ; }

	protected:
		virtual void			OnInitialize() override;

	private:
		Matrix					CreateProjectionMatrix() const;

	private:
		TransformComponent*		_ownerTransform = nullptr;

		CameraProjectionMode	_projectionMode = CameraProjectionMode::Orthographic;
		float					_orthographicWidth = 1280.f;
		float					_orthographicHeight = 720.f;
		float					_fovYRadians = 1.04719755f;
		float					_aspectRatio = 16.f / 9.f;
		float					_nearZ = 0.1f;
		float					_farZ = 1000.f;
	};
}
