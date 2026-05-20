#include "CameraManager.h"
#include "Camera.h"
#include "GameObject.h"
#include "MathUtil.h"
#include <cmath>

namespace gm
{
	void CameraManager::Tick(float deltaTime)
	{
		_viewInfo = CameraViewInfo();

		Camera* activeCamera = GetActiveCamera();
		GM_ASSERT_RETURN(activeCamera, "활성 카메라가 없습니다.");

		_viewInfo = activeCamera->GetViewInfo();

		if (_isPixelSnapEnabled)
		{
			_viewInfo.position.x = std::round(_viewInfo.position.x);
			_viewInfo.position.y = std::round(_viewInfo.position.y);
			_viewInfo.view = Math::CreateViewMatrix(_viewInfo.position, _viewInfo.rotation);
		}

		TickShake(deltaTime);
		ApplyShake(_viewInfo);
	}

	void CameraManager::TickShake(float deltaTime)
	{
		if (_remainShakeTime <= 0.f)
			return;

		_remainShakeTime = _remainShakeTime - deltaTime;
		_shakePhase += deltaTime;
	}

	void CameraManager::ApplyShake(CameraViewInfo& viewInfo) const
	{
		if (_remainShakeTime <= 0.f || _activeShakeDesc.duration <= 0.f)
			return;

		const float ratio = _remainShakeTime / _activeShakeDesc.duration;
		const float strength = _activeShakeDesc.strength * ratio;
		const Vector3 right = Math::GetRightVector(viewInfo.rotation);
		const Vector3 up = Math::GetUpVector(viewInfo.rotation);
		const Vector3 look = Math::GetLookVector(viewInfo.rotation);

		const Vector3 locationShake(
			std::sin(_shakePhase * _activeShakeDesc.locationSpeed.x) * _activeShakeDesc.locationAmplitude.x,
			std::cos(_shakePhase * _activeShakeDesc.locationSpeed.y) * _activeShakeDesc.locationAmplitude.y,
			std::sin(_shakePhase * _activeShakeDesc.locationSpeed.z) * _activeShakeDesc.locationAmplitude.z
		);

		const Vector3 rotationShake(
			std::sin(_shakePhase * _activeShakeDesc.rotationSpeed.x) * _activeShakeDesc.rotationAmplitude.x,
			std::cos(_shakePhase * _activeShakeDesc.rotationSpeed.y) * _activeShakeDesc.rotationAmplitude.y,
			std::sin(_shakePhase * _activeShakeDesc.rotationSpeed.z) * _activeShakeDesc.rotationAmplitude.z
		);

		const Vector3 localOffset = right * locationShake.x + up * locationShake.y + look * locationShake.z;

		const Quaternion rotationOffset = Quaternion::CreateFromYawPitchRoll(rotationShake.y, rotationShake.x, rotationShake.z);

		viewInfo.position += localOffset * strength;
		viewInfo.rotation = viewInfo.rotation * rotationOffset;
		viewInfo.view = Math::CreateViewMatrix(viewInfo.position, viewInfo.rotation);
	}

	void CameraManager::RegisterCamera(const std::wstring& cameraKey, Camera* camera)
	{
		GM_ASSERT_RETURN(camera, "카메라가 유효하지 않습니다.");
		GM_ASSERT_RETURN(cameraKey.empty() == false, "Camera Key가 비어 있습니다.");

		CameraEntry entry{};
		entry.owner = camera->GetOwner().GetWeakPtr();
		entry.camera = camera;

		_cameraList[cameraKey] = entry;

		if (_activeCamera == nullptr)
		{
			_activeCamera = camera;
			_activeCameraOwner = entry.owner;
		}
	}

	void CameraManager::UnregisterCamera(const std::wstring& cameraKey)
	{
		const auto iter = _cameraList.find(cameraKey);
		if (iter == _cameraList.end())
			return;

		if (_activeCamera == iter->second.camera)
		{
			_activeCamera = nullptr;
			_activeCameraOwner.Reset();
		}

		_cameraList.erase(cameraKey);
	}

	void CameraManager::UnregisterCamera(Camera* camera)
	{
		if (camera == nullptr)
			return;

		for (auto iter = _cameraList.begin(); iter != _cameraList.end();)
		{
			if (iter->second.camera != camera)
			{
				++iter;
				continue;
			}

			const bool isActiveCamera = _activeCamera == iter->second.camera;
			iter = _cameraList.erase(iter);

			if (isActiveCamera)
			{
				_activeCamera = nullptr;
				_activeCameraOwner.Reset();
			}
		}
	}

	void CameraManager::SetActiveCamera(const std::wstring& cameraKey)
	{
		const auto iter = _cameraList.find(cameraKey);
		GM_ASSERT_RETURN(iter != _cameraList.end(), "%ls Camera가 등록되어 있지 않습니다.", cameraKey.c_str());

		_activeCamera = iter->second.camera;
		_activeCameraOwner = iter->second.owner;
	}

	Camera* CameraManager::GetActiveCamera() const
	{
		if (_activeCamera == nullptr || _activeCameraOwner.IsValid() == false)
			return nullptr;

		return _activeCamera;
	}

	Vector2 CameraManager::WorldToScreen(const Vector3& worldPosition, uint32 screenWidth, uint32 screenHeight) const
	{
		const Matrix viewProjection = _viewInfo.view * _viewInfo.projection;

		Vector4 clipPosition(worldPosition.x, worldPosition.y, worldPosition.z, 1.f);
		clipPosition = Vector4::Transform(clipPosition, viewProjection);

		if (clipPosition.w == 0.f)
			return Vector2();

		const float ndcX = clipPosition.x / clipPosition.w;
		const float ndcY = clipPosition.y / clipPosition.w;

		return Vector2(
			(ndcX + 1.f) * 0.5f * static_cast<float>(screenWidth),
			(1.f - ndcY) * 0.5f * static_cast<float>(screenHeight)
		);
	}

	Vector2 CameraManager::WorldToScreen(const Vector2& worldPosition, uint32 screenWidth, uint32 screenHeight) const
	{
		return WorldToScreen(Vector3(worldPosition.x, worldPosition.y, 0.f), screenWidth, screenHeight);
	}

	void CameraManager::AddShake()
	{
		AddShake(_defaultShakeDesc);
	}

	void CameraManager::AddShake(float duration, float strength)
	{
		CameraShakeDesc desc = _defaultShakeDesc;
		desc.duration = duration;
		desc.strength = strength;
		AddShake(desc);
	}

	void CameraManager::AddShake(const CameraShakeDesc& desc)
	{
		_activeShakeDesc = desc;
		_remainShakeTime = desc.duration;
		_shakePhase = 0.f;
	}

	void CameraManager::StopShake()
	{
		_remainShakeTime = 0.f;
		_shakePhase = 0.f;
	}
}
