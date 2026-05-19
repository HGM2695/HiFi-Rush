#pragma once

#include "Component.h"
#include "CameraViewInfo.h"

namespace gm
{
	class Transform;

	enum class CameraProjectionMode
	{
		Orthographic,
		Perspective,
	};

	class Camera : public Component
	{
	public:
		Camera() = default;
		virtual ~Camera() override;
		virtual TickGroup		GetTickGroup() const override { return TickGroup::Camera; }

		CameraViewInfo			GetViewInfo() const;

		void					SetOrthographic(float width, float height, float nearZ = 0.f, float farZ = 1000.f);
		void					SetPerspective(float fovYRadians, float aspectRatio, float nearZ = 0.1f, float farZ = 1000.f);

	protected:
		virtual void			OnInitialize() override;

	private:
		Matrix					CreateProjectionMatrix() const;

	private:
		Transform*				_ownerTransform = nullptr;

		CameraProjectionMode	_projectionMode = CameraProjectionMode::Orthographic;
		float					_orthographicWidth = 1280.f;
		float					_orthographicHeight = 720.f;
		float					_fovYRadians = 1.04719755f;
		float					_aspectRatio = 16.f / 9.f;
		float					_nearZ = 0.1f;
		float					_farZ = 1000.f;
	};
}
