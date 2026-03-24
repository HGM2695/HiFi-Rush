#include "Camera.h"
#include "Transform.h"
#include "GameObject.h"
#include "GMAssert.h"
#include "Application.h"
#include "Time.h"

namespace gm
{
    Camera::~Camera()
    {
        if (_mainCamera == this)
            _mainCamera = nullptr;
    }

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
        return worldPos - _cameraPosition + math::Vector2(APPLICATION.GetWidth() * 0.5f, APPLICATION.GetHeight() * 0.5f);
    }

    void Camera::OnInitialize()
    {
        _ownerTransform = GetOwner().GetComponent<Transform>();
        _cameraPosition = _ownerTransform->GetPosition();

        if (_mainCamera == nullptr)
            Camera::SetMainCamera(this);
    }

    void Camera::OnLateUpdate()
    {
        FollowOwner();
    }

    void Camera::FollowOwner()
    {
        math::Vector2 ownerPosition = _ownerTransform->GetPosition();
        math::Vector2 targetPosition = _cameraPosition;

        const float gapX = ownerPosition._x - _cameraPosition._x;
        const float gapY = ownerPosition._y - _cameraPosition._y;
        const float halfWidth = _deadZoneWidth * 0.5f;
        const float halfHeight = _deadZoneHeight * 0.5f;

        if (gapX > halfWidth)
            targetPosition._x = ownerPosition._x - halfWidth;
        else if (gapX < -halfWidth)
            targetPosition._x = ownerPosition._x + halfWidth;
        if (gapY > halfHeight)
            targetPosition._y = ownerPosition._y - halfHeight;
        else if (gapY < -halfHeight)
            targetPosition._y = ownerPosition._y + halfHeight;

        float deltaTime = APPLICATION.GetTime().GetDeltaTime();
        float t = 1.f - std::exp(-_followSpeed * deltaTime);

        _cameraPosition = math::Vector2::LerpClamped(_cameraPosition, targetPosition, t);
    }
}
