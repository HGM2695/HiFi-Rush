#include "ChiEffectComponent.h"

#include "ChiAfterImageObject.h"
#include "CameraComponent.h"
#include "CameraFollowComponent.h"
#include "CameraManager.h"
#include "EffectPresets.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "PlayerResources.h"
#include "Resources.h"
#include "Scene.h"
#include "SkeletalMeshComponent.h"
#include "SocketComponent.h"
#include "TransformComponent.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr wchar_t JumpEffectId[] = L"Chi.Jump";
		constexpr wchar_t FingerSnapEffectId[] = L"Chi.Idle.FingerSnap";
		constexpr wchar_t JumpTrailEffectId[] = L"Chi.JumpTrail";
		constexpr wchar_t LandingEffectId[] = L"Chi.Landing";
		constexpr wchar_t DoubleJumpEffectId[] = L"Chi.DoubleJump";
		constexpr wchar_t DashEffectId[] = L"Chi.Dash";
		constexpr std::array<const wchar_t*, 3> DoubleDashCrescentEffectIds = { L"Chi.Dash.Double.Blue", L"Chi.Dash.Double.Yellow", L"Chi.Dash.Double.Orange" };
		constexpr wchar_t StumpEffectId[] = L"Chi.Stump";
		constexpr wchar_t HibikiCloudEffectId[] = L"Chi.Hibiki.Cloud";
		constexpr wchar_t HibikiExitCloudEffectId[] = L"Chi.Hibiki.ExitClouds";
		constexpr float AfterImageTrailDuration = 0.45f;
		constexpr float AfterImageLifetime = 0.75f;
		constexpr float AfterImageDelayScale = 0.8f;
		constexpr std::array<float, 3> AfterImageMaximumDelays = { 10.f / 120.f, 12.f / 120.f, 14.f / 120.f };
		constexpr std::array<float, 3> AfterImageCameraRightOffsets = { 0.f, -0.1f, 0.1f };
		std::wstring GetAttackEffectId(ChiAnimationClipId animationClipId)
		{
			return std::wstring(L"Chi.Attack.") + GetChiAnimationClipName(animationClipId);
		}

		std::wstring GetBeatHitEffectId(ChiAnimationClipId animationClipId)
		{
			return std::wstring(L"Chi.BeatHit.") + GetChiAnimationClipName(animationClipId);
		}
	}

	ChiEffectComponent::ChiEffectComponent(Resources& resources, const EffectPresets& effectPresets)
		: _effectSpawner(resources, effectPresets)
	{
	}

	void ChiEffectComponent::OnInitialize()
	{
		_skeletalMeshComponent = GetOwner().GetComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN(_skeletalMeshComponent, "ChiEffectComponent는 SkeletalMeshComponent가 필요합니다.");
	}

	void ChiEffectComponent::OnTick(float deltaTime)
	{
		SpawnPendingFingerSnapEffect();
		if (_isAfterImageTrailActive == false)
			return;

		_afterImageElapsed += std::max(0.f, deltaTime);
		UpdateAfterImages();
	}

	void ChiEffectComponent::SpawnJumpEffect()
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Jump Effect를 생성할 Scene이 없습니다.");
		const Matrix spawnTransform = Matrix::CreateTranslation(GetOwner().GetTransform()->GetPosition());
		_effectSpawner.SpawnAtWorld(*scene, JumpEffectId, spawnTransform);
		_effectSpawner.SpawnAttachedToOwner(*scene, JumpTrailEffectId, GetOwner());
	}

	void ChiEffectComponent::RequestFingerSnapEffect()
	{
		_isFingerSnapEffectPending = true;
	}

	void ChiEffectComponent::SpawnPendingFingerSnapEffect()
	{
		if (_isFingerSnapEffectPending == false)
			return;
		_isFingerSnapEffectPending = false;

		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Finger Snap Effect를 생성할 Scene이 없습니다.");
		SocketComponent* socketComponent = GetOwner().GetComponent<SocketComponent>();
		GM_ASSERT_RETURN(socketComponent && socketComponent->HasSocket(ChiFingerSnapSocketName), "Chi Finger Snap Effect Socket을 찾을 수 없습니다.");
		const Matrix& socketWorld = socketComponent->GetSocketAnchorWorldMatrix(ChiFingerSnapSocketName);
		const Vector3 socketPosition = Vector3::Transform(Vector3::Zero, socketWorld);
		GM_ASSERT(_effectSpawner.SpawnAtWorld(*scene, FingerSnapEffectId, Matrix::CreateTranslation(socketPosition)), "Chi Finger Snap Effect 생성에 실패했습니다.");
	}

	void ChiEffectComponent::SpawnLandingEffect()
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Landing Effect를 생성할 Scene이 없습니다.");
		const Matrix spawnTransform = Matrix::CreateTranslation(GetOwner().GetTransform()->GetPosition());
		_effectSpawner.SpawnAtWorld(*scene, LandingEffectId, spawnTransform);
	}

	void ChiEffectComponent::SpawnDoubleJumpEffect()
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Double Jump Effect를 생성할 Scene이 없습니다.");
		_effectSpawner.SpawnAttachedToOwner(*scene, DoubleJumpEffectId, GetOwner());
	}

	void ChiEffectComponent::SpawnDashEffect(const Vector3& dashDirection, float cameraDistanceOffset)
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Dash Effect를 생성할 Scene이 없습니다.");
		const Matrix ownerWorldInverse = GetOwner().GetTransform()->GetWorldMatrix().Invert();
		const Vector3 localDashDirection = Vector3::TransformNormal(dashDirection, ownerWorldInverse);
		const Matrix dashLocalRotation = Matrix::CreateFromQuaternion(Math::CreateRotationByDirection(localDashDirection));
		_effectSpawner.SpawnAttachedToOwner(*scene, DashEffectId, GetOwner(), dashLocalRotation);
		StartAfterImageTrail();
		StartDashCamera(cameraDistanceOffset);
	}

	void ChiEffectComponent::SpawnDoubleDashCrescent(uint32 crescentIndex)
	{
		GM_ASSERT_RETURN(crescentIndex < DoubleDashCrescentEffectIds.size(), "지원하지 않는 Double Dash Crescent Index입니다.");
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Double Dash Crescent Effect를 생성할 Scene이 없습니다.");
		_effectSpawner.SpawnAttachedToOwner(*scene, DoubleDashCrescentEffectIds[crescentIndex], GetOwner());
	}

	void ChiEffectComponent::SpawnDashLandingEffect(float forwardOffset)
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Dash Landing Effect를 생성할 Scene이 없습니다.");
		TransformComponent* transform = GetOwner().GetTransform();
		const Vector3 spawnPosition = transform->GetPosition() + Math::GetLookVector(transform->GetRotation()) * forwardOffset;
		_effectSpawner.SpawnAtWorld(*scene, LandingEffectId, Matrix::CreateTranslation(spawnPosition));
	}

	void ChiEffectComponent::EndDashEffect()
	{
		StopAfterImageTrail();
		if (_cameraFollowComponent && _dashCameraBaseDistance)
			_cameraFollowComponent->SetTargetDistance(_dashCameraBaseDistance.value());
		_cameraFollowComponent = nullptr;
		_dashCameraBaseDistance.reset();
	}

	void ChiEffectComponent::SetAfterImageEmissiveIntensity(float intensity)
	{
		_afterImageEmissiveIntensity = std::max(0.f, intensity);
		for (uint32 index = 0; index < _afterImages.size(); ++index)
		{
			ChiAfterImageObject* afterImage = GetAfterImage(index);
			if (afterImage)
				afterImage->SetEmissiveIntensity(_afterImageEmissiveIntensity);
		}
	}

	void ChiEffectComponent::SetAfterImageColor(uint32 index, const Color& color)
	{
		GM_ASSERT_RETURN(index < _afterImageColors.size(), "지원하지 않는 After Image Color Index입니다.");
		_afterImageColors[index] = Color{ color.x, color.y, color.z, _afterImageOpacity };
		ChiAfterImageObject* afterImage = GetAfterImage(index);
		if (afterImage)
			afterImage->SetColor(_afterImageColors[index]);
	}

	Color ChiEffectComponent::GetAfterImageColor(uint32 index) const
	{
		GM_ASSERT_RETURN_VAL(index < _afterImageColors.size(), Colors::White, "지원하지 않는 After Image Color Index입니다.");
		return _afterImageColors[index];
	}

	void ChiEffectComponent::SetAfterImageOpacity(float opacity)
	{
		_afterImageOpacity = std::clamp(opacity, 0.f, 1.f);
		for (uint32 index = 0; index < _afterImageColors.size(); ++index)
		{
			_afterImageColors[index].w = _afterImageOpacity;
			ChiAfterImageObject* afterImage = GetAfterImage(index);
			if (afterImage)
				afterImage->SetColor(_afterImageColors[index]);
		}
	}

	void ChiEffectComponent::SetAfterImageColorBlendRatio(float ratio)
	{
		_afterImageColorBlendRatio = std::clamp(ratio, 0.f, 1.f);
		for (uint32 index = 0; index < _afterImages.size(); ++index)
		{
			ChiAfterImageObject* afterImage = GetAfterImage(index);
			if (afterImage)
				afterImage->SetColorBlendRatio(_afterImageColorBlendRatio);
		}
	}

	void ChiEffectComponent::SpawnAttackEffect(ChiAnimationClipId animationClipId)
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Attack Effect를 생성할 Scene이 없습니다.");
		const std::wstring effectId = GetAttackEffectId(animationClipId);
		_effectSpawner.SpawnAttachedToOwner(*scene, effectId, GetOwner());
	}

	void ChiEffectComponent::SpawnBeatHitEffect(ChiAnimationClipId animationClipId, const Matrix& world)
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Beat Hit Effect를 생성할 Scene이 없습니다.");
		const std::wstring effectId = GetBeatHitEffectId(animationClipId);
		_effectSpawner.SpawnAtWorld(*scene, effectId, world);
	}

	void ChiEffectComponent::SpawnStumpEffect()
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Stump Effect를 생성할 Scene이 없습니다.");
		_effectSpawner.SpawnAtWorld(*scene, StumpEffectId, GetOwner().GetTransform()->GetWorldMatrix());
	}

	void ChiEffectComponent::SpawnHibikiCloud()
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Hibiki Cloud Effect를 생성할 Scene이 없습니다.");
		_effectSpawner.SpawnAttachedToOwner(*scene, HibikiCloudEffectId, GetOwner());
	}

	void ChiEffectComponent::SpawnHibikiExitClouds()
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN(scene, "Chi Hibiki Exit Cloud Effect를 생성할 Scene이 없습니다.");
		_effectSpawner.SpawnAttachedToOwner(*scene, HibikiExitCloudEffectId, GetOwner());
	}

	void ChiEffectComponent::StartAfterImageTrail()
	{
		StopAfterImageTrail();
		if (_skeletalMeshComponent == nullptr)
			return;

		Scene* scene = GetOwner().GetScene();
		if (scene == nullptr)
			return;

		_afterImageElapsed = 0.f;
		_afterImageHistory.reserve(32);
		_afterImageHistory.push_back(AfterImageSample{ 0.f, GetOwner().GetTransform()->GetWorldMatrix() });
		for (uint32 index = 0; index < _afterImages.size(); ++index)
		{
			ChiAfterImageObjectDesc desc{};
			desc.sourceMeshComponent = _skeletalMeshComponent;
			desc.world = GetOwner().GetTransform()->GetWorldMatrix();
			desc.color = _afterImageColors[index];
			desc.lifetime = AfterImageLifetime;
			desc.emissiveIntensity = _afterImageEmissiveIntensity;
			desc.colorBlendRatio = _afterImageColorBlendRatio;
			desc.fadeOut = false;
			ChiAfterImageObject* afterImage = scene->SpawnGameObject<ChiAfterImageObject>(desc);
			if (afterImage == nullptr)
			{
				StopAfterImageTrail();
				return;
			}
			afterImage->SetRender(false);
			_afterImages[index] = afterImage->GetWeakPtr();
		}
		_isAfterImageTrailActive = true;
	}

	void ChiEffectComponent::UpdateAfterImages()
	{
		if (_skeletalMeshComponent == nullptr || _afterImageHistory.empty())
		{
			StopAfterImageTrail();
			return;
		}

		_afterImageHistory.push_back(AfterImageSample{ _afterImageElapsed, GetOwner().GetTransform()->GetWorldMatrix() });
		const float trailRatio = std::clamp(_afterImageElapsed / AfterImageTrailDuration, 0.f, 1.f);
		const float delayRatio = std::sin(Math::DegreesToRadians(180.f) * trailRatio);
		Scene* scene = GetOwner().GetScene();
		CameraManager* cameraManager = scene ? scene->GetCameraManager() : nullptr;
		CameraComponent* activeCamera = cameraManager ? cameraManager->GetActiveCamera() : nullptr;
		const Vector3 cameraRight = activeCamera ? activeCamera->GetRightDirection() : Vector3::Right;
		for (uint32 index = 0; index < _afterImages.size(); ++index)
		{
			ChiAfterImageObject* afterImage = GetAfterImage(index);
			if (afterImage == nullptr)
				continue;

			const float targetElapsed = std::max(0.f, _afterImageElapsed - AfterImageMaximumDelays[index] * delayRatio * AfterImageDelayScale);
			const AfterImageSample* closestSample = &_afterImageHistory.front();
			float closestDifference = std::abs(closestSample->elapsed - targetElapsed);
			for (const AfterImageSample& sample : _afterImageHistory)
			{
				const float difference = std::abs(sample.elapsed - targetElapsed);
				if (difference >= closestDifference)
					continue;
				closestSample = &sample;
				closestDifference = difference;
			}
			const Matrix worldOffset = Matrix::CreateTranslation(cameraRight * AfterImageCameraRightOffsets[index]);
			afterImage->UpdateSnapshot(*_skeletalMeshComponent, closestSample->world * worldOffset);
		}

		if (_afterImageElapsed >= AfterImageTrailDuration)
			StopAfterImageTrail();
	}

	void ChiEffectComponent::StopAfterImageTrail()
	{
		for (WeakGameObjectPtr& afterImagePtr : _afterImages)
		{
			ChiAfterImageObject* afterImage = dynamic_cast<ChiAfterImageObject*>(afterImagePtr.Get());
			if (afterImage)
				afterImage->Finish();
			afterImagePtr.Reset();
		}
		_afterImageHistory.clear();
		_afterImageElapsed = 0.f;
		_isAfterImageTrailActive = false;
	}

	ChiAfterImageObject* ChiEffectComponent::GetAfterImage(uint32 index) const
	{
		if (index >= _afterImages.size())
			return nullptr;
		return dynamic_cast<ChiAfterImageObject*>(_afterImages[index].Get());
	}

	void ChiEffectComponent::StartDashCamera(float distanceOffset)
	{
		if (distanceOffset <= 0.f)
			return;

		_cameraFollowComponent = FindCameraFollowComponent();
		if (_cameraFollowComponent == nullptr)
			return;
		_dashCameraBaseDistance = _cameraFollowComponent->GetTargetDistance();
		_cameraFollowComponent->SetTargetDistance(_dashCameraBaseDistance.value() + distanceOffset);
	}

	CameraFollowComponent* ChiEffectComponent::FindCameraFollowComponent() const
	{
		Scene* scene = GetOwner().GetScene();
		CameraManager* cameraManager = scene ? scene->GetCameraManager() : nullptr;
		CameraComponent* activeCamera = cameraManager ? cameraManager->GetActiveCamera() : nullptr;
		return activeCamera ? activeCamera->GetOwner().GetComponent<CameraFollowComponent>() : nullptr;
	}
}
