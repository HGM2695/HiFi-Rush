#include "Camera.h"
#include "Transform.h"
#include "GameObject.h"
#include "Application.h"
#include "TimeSystem.h"
#include <algorithm>
#include <cmath>

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

    Vector2 Camera::MainWorldToScreen(const Vector2& worldPos)
    {
        GM_ASSERT_RETURN_VAL(_mainCamera, Vector2(), "메인 카메라가 존재하지 않습니다.");
        return _mainCamera->WorldToScreen(worldPos);
    }

    Vector2 Camera::WorldToScreen(const Vector2& worldPos) const
    {
        const Vector2 cameraSpacePos = worldPos - _cameraPosition;
        // 카메라의 위치를 화면 중앙에 고정한다는 의미.
        return Vector2(APPLICATION.GetWidth() * 0.5f + cameraSpacePos.x, APPLICATION.GetHeight() * 0.5f - cameraSpacePos.y);
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
        Vector2 ownerPosition = _ownerTransform->GetPosition();
        Vector2 targetPosition = _cameraPosition;

        const float gapX = ownerPosition.x - _cameraPosition.x;
        const float gapY = ownerPosition.y - _cameraPosition.y;
        const float halfWidth = _deadZoneWidth * 0.5f;
        const float halfHeight = _deadZoneHeight * 0.5f;

        if (gapX > halfWidth)
            targetPosition.x = ownerPosition.x - halfWidth;
        else if (gapX < -halfWidth)
            targetPosition.x = ownerPosition.x + halfWidth;
        if (gapY > halfHeight)
            targetPosition.y = ownerPosition.y - halfHeight;
        else if (gapY < -halfHeight)
            targetPosition.y = ownerPosition.y + halfHeight;

        float deltaTime = APPLICATION.GetTimeSystem().GetDeltaTime();
        float t = 1.f - std::exp(-_followSpeed * deltaTime);

        t = std::clamp(t, 0.f, 1.f);
        _cameraPosition = _cameraPosition + (targetPosition - _cameraPosition) * t;
    }
}
