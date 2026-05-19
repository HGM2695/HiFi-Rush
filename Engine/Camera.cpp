#include "Camera.h"
#include "Transform.h"
#include "GameObject.h"
#include "MathUtil.h"

namespace gm
{
	Camera::~Camera() = default;

	void Camera::OnInitialize()
	{
		_ownerTransform = GetOwner().GetComponent<Transform>();
		GM_ASSERT_RETURN(_ownerTransform, "Camera 소유자의 Transform이 존재하지 않습니다.");
	}

	CameraViewInfo Camera::GetViewInfo() const
	{
		CameraViewInfo viewInfo{};
		viewInfo.position = _ownerTransform->GetPosition();
		viewInfo.rotation = _ownerTransform->GetRotation();
		viewInfo.view = Math::CreateViewMatrix(viewInfo.position, viewInfo.rotation);
		viewInfo.projection = CreateProjectionMatrix();

		return viewInfo;
	}

	void Camera::SetOrthographic(float width, float height, float nearZ, float farZ)
	{
		_projectionMode = CameraProjectionMode::Orthographic;
		_orthographicWidth = width;
		_orthographicHeight = height;
		_nearZ = nearZ;
		_farZ = farZ;
	}

	void Camera::SetPerspective(float fovYRadians, float aspectRatio, float nearZ, float farZ)
	{
		_projectionMode = CameraProjectionMode::Perspective;
		_fovYRadians = fovYRadians;
		_aspectRatio = aspectRatio;
		_nearZ = nearZ;
		_farZ = farZ;
	}

	Matrix Camera::CreateProjectionMatrix() const
	{
		switch (_projectionMode)
		{
		case CameraProjectionMode::Orthographic:
			return Matrix::CreateOrthographic(_orthographicWidth, _orthographicHeight, _nearZ, _farZ);
		case CameraProjectionMode::Perspective:
			return Matrix::CreatePerspectiveFieldOfView(_fovYRadians, _aspectRatio, _nearZ, _farZ);
		default:
			GM_ASSERT_RETURN_VAL(false, Math::IdentityMatrix(), "지원하지 않는 카메라 투영 모드입니다.");
		}
	}
}
