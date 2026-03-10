#include "Camera.h"
#include "Transform.h"
#include "GameObject.h"
#include "GMAssert.h"

namespace gm
{
	Camera* Camera::GetMainCamera()
	{
		return _mainCamera;
	}

	void Camera::SetMainCamera(Camera* camera)
	{
		GM_ASSERT_RETURN(camera, "카메라가 nullptr 입니다.");
		_mainCamera = camera;
	}

	math::Vector2 Camera::MainWorldToScreen(const math::Vector2& worldPos)
	{
		GM_ASSERT_RETURN_VAL(_mainCamera, math::Vector2(), "메인 카메라가 존재하지 않습니다.");
		return _mainCamera->WorldToScreen(worldPos);
	}

	math::Vector2 Camera::WorldToScreen(const math::Vector2& worldPos) const
	{
		return worldPos - _transform->GetPosition();
	}

	void Camera::OnInitialize()
	{
		_transform = GetOwner().GetComponent<Transform>();

		if (_mainCamera == nullptr)
			Camera::SetMainCamera(this);
	}

	void Camera::OnLateUpdate()
	{

	}
}