#pragma once

#include "EngineCore.h"
#include "CameraViewInfo.h"
#include "WeakGameObjectPtr.h"
#include <unordered_map>

namespace gm
{
	class Camera;

	struct CameraShakeDesc
	{
		float	duration = 0.2f;
		float	strength = 1.f;

		Vector3	locationAmplitude = Vector3(4.f, 3.f, 0.f);
		Vector3	rotationAmplitude = Vector3(0.01f, 0.008f, 0.02f);

		Vector3	locationSpeed = Vector3(21.f, 17.f, 13.f);
		Vector3	rotationSpeed = Vector3(19.f, 23.f, 29.f);
	};

	class CameraManager
	{
	public:
		void				Tick(float deltaTime);
		void				RegisterCamera(const std::wstring& cameraKey, Camera* camera);
		void				UnregisterCamera(const std::wstring& cameraKey);
		void				UnregisterCamera(Camera* camera);
		void				SetActiveCamera(const std::wstring& cameraKey);
		Camera*				GetActiveCamera() const;
		CameraViewInfo		GetViewInfo() const { return _viewInfo; }
		Vector2				WorldToScreen(const Vector3& worldPosition, uint32 screenWidth, uint32 screenHeight) const;
		Vector2				WorldToScreen(const Vector2& worldPosition, uint32 screenWidth, uint32 screenHeight) const;

		void				SetDefaultShakeDesc(const CameraShakeDesc& desc) { _defaultShakeDesc = desc; }
		void				AddShake();
		void				AddShake(float duration, float strength = 1.f);
		void				AddShake(const CameraShakeDesc& desc);
		void				StopShake();

	private:
		void				TickShake(float deltaTime);
		void				ApplyShake(CameraViewInfo& viewInfo) const;

	private:
		struct CameraEntry
		{
			WeakGameObjectPtr owner;
			Camera* camera = nullptr;
		};

		std::unordered_map<std::wstring, CameraEntry>	_cameraList{};
		Camera*										_activeCamera = nullptr;
		WeakGameObjectPtr							_activeCameraOwner{};
		CameraViewInfo								_viewInfo{};

		CameraShakeDesc								_defaultShakeDesc{};
		CameraShakeDesc								_activeShakeDesc{};
		float										_remainShakeTime = 0.f;
		float										_shakePhase = 0.f;
	};
}
