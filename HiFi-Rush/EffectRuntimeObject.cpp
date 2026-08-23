#include "EffectRuntimeObject.h"

#include "BeatSystem.h"
#include "BuiltinGraphicsResources.h"
#include "EffectSpriteComponent.h"
#include "HiFiRushStatics.h"
#include "Material.h"
#include "MathUtil.h"
#include "Resources.h"
#include "Random.h"
#include "Shader.h"
#include "SocketComponent.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "Texture.h"
#include "TransformComponent.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr uint32 EffectMaterialConstantSlot = 2;
	}

	EffectRuntimeObject::EffectRuntimeObject(const EffectRuntimeObjectDesc& desc)
		: _delay(desc.track.delay)
		, _lifetime(desc.track.lifetime)
		, _attachmentMode(desc.attachmentMode)
		, _localTransform(desc.track.localTransform)
		, _spawnTransform(desc.spawnTransform)
		, _ownerSpawnPosition(desc.ownerSpawnPosition)
		, _startScale(desc.track.startScale)
		, _endScale(desc.track.endScale)
		, _endPositionOffset(desc.track.endPositionOffset)
		, _endRotationOffsetDegrees(desc.track.endRotationOffsetDegrees)
		, _currentScale(desc.track.startScale)
		, _positionInterpolationSpeed(desc.track.positionInterpolationSpeed)
		, _rotationInterpolationSpeed(desc.track.rotationInterpolationSpeed)
		, _scaleInterpolationSpeed(desc.track.scaleInterpolationSpeed)
		, _scaleInterpolationDelay(desc.track.scaleInterpolationDelay)
		, _startAngularVelocityDegrees(desc.track.startAngularVelocityDegrees)
		, _endAngularVelocityDegrees(desc.track.endAngularVelocityDegrees)
		, _currentAngularVelocityDegrees(desc.track.startAngularVelocityDegrees)
		, _angularVelocityInterpolationSpeed(desc.track.angularVelocityInterpolationSpeed)
		, _animationType(desc.track.animationType)
		, _animationSpeed(desc.track.animationSpeed)
		, _owner(desc.owner)
		, _socketName(desc.socketName)
		, _spriteSheetColumns(desc.track.spriteSheetColumns)
		, _spriteSheetRows(desc.track.spriteSheetRows)
		, _spriteSheetStartFrame(desc.track.spriteSheetStartFrame)
		, _spriteSheetFrameCount(desc.track.spriteSheetFrameCount)
		, _spriteSheetFrameDuration(desc.track.spriteSheetFrameDuration)
		, _startOpacity(desc.track.opacity)
		, _endOpacity(desc.track.endOpacity)
		, _currentOpacity(desc.track.opacity)
		, _opacityInterpolationSpeed(desc.track.opacityInterpolationSpeed)
		, _startFillRatio(desc.track.startFillRatio)
		, _endFillRatio(desc.track.endFillRatio)
		, _radialCenter(desc.track.radialCenter)
		, _radialStartAngle(desc.track.radialStartAngle)
		, _radialSweepAngle(desc.track.radialSweepAngle)
		, _emissiveDuration(desc.track.emissiveDuration)
		, _dissolveStartThreshold(desc.track.dissolveStartThreshold)
		, _dissolveEndThreshold(desc.track.dissolveEndThreshold)
		, _hasDissolve(desc.dissolveTexture != nullptr)
	{
		_randomPositionOffset = Vector3{ Math::RandomFloat(desc.track.randomPositionMinimum.x, desc.track.randomPositionMaximum.x), Math::RandomFloat(desc.track.randomPositionMinimum.y, desc.track.randomPositionMaximum.y), Math::RandomFloat(desc.track.randomPositionMinimum.z, desc.track.randomPositionMaximum.z) };
		_randomRotationOffsetDegrees = Vector3{ Math::RandomFloat(desc.track.randomRotationMinimumDegrees.x, desc.track.randomRotationMaximumDegrees.x), Math::RandomFloat(desc.track.randomRotationMinimumDegrees.y, desc.track.randomRotationMaximumDegrees.y), Math::RandomFloat(desc.track.randomRotationMinimumDegrees.z, desc.track.randomRotationMaximumDegrees.z) };
		_endPositionOffset += Vector3{ Math::RandomFloat(desc.track.randomEndPositionOffsetMinimum.x, desc.track.randomEndPositionOffsetMaximum.x), Math::RandomFloat(desc.track.randomEndPositionOffsetMinimum.y, desc.track.randomEndPositionOffsetMaximum.y), Math::RandomFloat(desc.track.randomEndPositionOffsetMinimum.z, desc.track.randomEndPositionOffsetMaximum.z) };
		_endScale += Vector3::Lerp(desc.track.randomEndScaleMinimum, desc.track.randomEndScaleMaximum, Math::RandomFloat(0.f, 1.f));
		SetRender(false);
		_isConfigured = ConfigureVisual(desc);
	}

	void EffectRuntimeObject::SetWorldTransform(const Matrix& world)
	{
		GM_ASSERT_RETURN(_attachmentMode == EffectAttachmentMode::World, "World Effect만 World Transform을 변경할 수 있습니다.");
		_spawnTransform = world;
		if (IsInitialized())
			UpdateAttachmentTransform();
	}

	void EffectRuntimeObject::OnInitialize()
	{
		if (_isConfigured == false || UpdateAttachmentTransform() == false)
		{
			Destroy();
			return;
		}

		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (beatSystem.HasPlaybackTime())
			_previousBeat = beatSystem.GetCurrentBeat();

		if (_delay.value <= 0.f)
			Activate();
	}

	void EffectRuntimeObject::OnTick(float deltaTime)
	{
		UpdateElapsedTime(deltaTime);
		UpdateTransformAnimation(deltaTime);
		if (UpdateAttachmentTransform() == false)
		{
			Destroy();
			return;
		}

		if (_isActive == false)
		{
			if (_delay.HasElapsed(_elapsedSeconds, _elapsedBeats))
				Activate();

			return;
		}

		UpdatePresentation(deltaTime);
		if (_lifetime.HasElapsed(_activeElapsedSeconds, _activeElapsedBeats))
			Destroy();
	}

	bool EffectRuntimeObject::ConfigureVisual(const EffectRuntimeObjectDesc& desc)
	{
		_effectMaterialConstant.emissiveColor = desc.track.emissiveColor;
		_effectMaterialConstant.emissiveIntensity = desc.track.emissiveIntensity;
		if (desc.track.visualType == EffectVisualType::Sprite)
		{
			GM_ASSERT_RETURN_VAL(desc.spriteTexture, false, "Effect Sprite Texture가 로드되지 않았습니다.");
			_spriteComponent = AddComponent<EffectSpriteComponent>();
			GM_ASSERT_RETURN_VAL(_spriteComponent, false, "EffectSpriteComponent 생성에 실패했습니다.");
			_spriteComponent->SetTexture(desc.spriteTexture);
			_spriteComponent->SetFacingMode(desc.track.facingMode);
			_spriteComponent->SetSortDepthOffset(desc.track.sortDepthOffset);
			_spriteComponent->SetOpacity(desc.track.opacity);
			_spriteComponent->SetEmissive(desc.track.emissiveColor, desc.track.emissiveIntensity);
			_spriteComponent->SetFillRatio(desc.track.startFillRatio);
			if (desc.track.radialSweepAngle > 0.f)
			{
				const bool isRotationCircle = desc.track.animationType == EffectAnimationType::RotationCircle || desc.track.animationType == EffectAnimationType::ContinuousRotationCircle;
				_spriteComponent->SetRadialFill(desc.track.radialCenter, desc.track.radialStartAngle, isRotationCircle ? 0.f : desc.track.radialSweepAngle);
			}

			if (desc.dissolveTexture)
			{
				_spriteComponent->SetDissolveTexture(desc.dissolveTexture);
				_spriteComponent->SetDissolveThreshold(desc.track.dissolveStartThreshold);
			}

			return true;
		}

		GM_ASSERT_RETURN_VAL(desc.track.visualType == EffectVisualType::StaticMesh, false, "지원하지 않는 Effect Visual Type입니다.");
		GM_ASSERT_RETURN_VAL(desc.staticMesh && desc.effectMeshShader, false, "Effect StaticMesh 또는 Effect Mesh Shader가 로드되지 않았습니다.");
		_staticMeshComponent = AddComponent<StaticMeshComponent>();
		GM_ASSERT_RETURN_VAL(_staticMeshComponent, false, "Effect StaticMeshComponent 생성에 실패했습니다.");
		_staticMeshComponent->SetStaticMesh(desc.staticMesh);
		_staticMeshComponent->SetCastsShadow(false);

		_effectMaterialConstant.dissolveThreshold = desc.track.dissolveStartThreshold;
		_effectMaterialConstant.dissolveEnabled = desc.dissolveTexture ? 1u : 0u;
		for (uint32 slotIndex = 0; slotIndex < desc.staticMesh->GetMaterialSlotCount(); ++slotIndex)
		{
			Material* material = _staticMeshComponent->GetMaterial(slotIndex);
			if (material == nullptr)
				continue;
			material->SetShadingModel(ShadingModel::Unlit);
			material->SetSurfaceMode(SurfaceMode::Transparent);
			material->SetOutlineMode(OutlineMode::Disabled);
			material->SetPixelShader(desc.effectMeshShader);
			RasterizerDesc rasterizerDesc = material->GetRasterizerDesc();
			rasterizerDesc.cullMode = CullMode::None;
			material->SetRasterizerDesc(rasterizerDesc);
			if (desc.baseColorTexture)
				material->SetTexture(TextureSlot::BaseColor, desc.baseColorTexture);
			material->SetTexture(TextureSlot::Custom0, desc.dissolveTexture);
			material->SetConstantData(ShaderStage::Pixel, EffectMaterialConstantSlot, _effectMaterialConstant);
		}
		return true;
	}

	void EffectRuntimeObject::UpdateTransformAnimation(float deltaTime)
	{
		if (_isActive == false)
			return;
		if (_animationType == EffectAnimationType::RotationCircle || _animationType == EffectAnimationType::ContinuousRotationCircle)
		{
			_currentRotationOffsetDegrees.z = Math::RadiansToDegrees(CalculateRotationCircleAngle());
			return;
		}

		if (_animationType == EffectAnimationType::BeatAcceleratedInterpolation)
		{
			const float frameRatio = std::clamp(_activeElapsedBeats * _animationSpeed, 0.f, 1.f);
			const float interpolationRatio = frameRatio >= 1.f ? 1.f : 1.f - std::pow(1.f - frameRatio, std::max(0.f, deltaTime) * 60.f);
			_currentScale = Vector3::Lerp(_currentScale, _endScale, interpolationRatio);
			_currentPositionOffset = Vector3::Lerp(_currentPositionOffset, _endPositionOffset, interpolationRatio);
			return;
		}

		const float safeDeltaTime = std::max(0.f, deltaTime);
		const float activeRatio = GetActiveRatio();
		if (_positionInterpolationSpeed > 0.f)
			_currentPositionOffset = Vector3::Lerp(_currentPositionOffset, _endPositionOffset, 1.f - std::exp(-_positionInterpolationSpeed * safeDeltaTime));
		else
			_currentPositionOffset = Vector3::Lerp(Vector3::Zero, _endPositionOffset, activeRatio);
		if (_scaleInterpolationDelay.HasElapsed(_activeElapsedSeconds, _activeElapsedBeats))
		{
			if (_scaleInterpolationSpeed > 0.f)
				_currentScale = Vector3::Lerp(_currentScale, _endScale, 1.f - std::exp(-_scaleInterpolationSpeed * safeDeltaTime));
			else
				_currentScale = Vector3::Lerp(_startScale, _endScale, activeRatio);
		}
		if (_startAngularVelocityDegrees != Vector3::Zero || _endAngularVelocityDegrees != Vector3::Zero)
		{
			const float velocityRatio = _angularVelocityInterpolationSpeed > 0.f ? 1.f - std::exp(-_angularVelocityInterpolationSpeed * safeDeltaTime) : activeRatio;
			_currentAngularVelocityDegrees = Vector3::Lerp(_currentAngularVelocityDegrees, _endAngularVelocityDegrees, velocityRatio);
			_currentRotationOffsetDegrees += _currentAngularVelocityDegrees * safeDeltaTime;
		}
		else if (_rotationInterpolationSpeed > 0.f)
		{
			_currentRotationOffsetDegrees = Vector3::Lerp(_currentRotationOffsetDegrees, _endRotationOffsetDegrees, 1.f - std::exp(-_rotationInterpolationSpeed * safeDeltaTime));
		}
		else
		{
			_currentRotationOffsetDegrees = Vector3::Lerp(Vector3::Zero, _endRotationOffsetDegrees, activeRatio);
		}
	}

	bool EffectRuntimeObject::UpdateAttachmentTransform()
	{
		Vector3 relativeScale{};
		Quaternion relativeRotation{};
		Vector3 relativePosition{};
		GM_ASSERT_RETURN_VAL(_localTransform.Decompose(relativeScale, relativeRotation, relativePosition), false, "Effect Local Transform을 분해할 수 없습니다.");
		relativeScale *= _currentScale;
		relativePosition += _currentPositionOffset + _randomPositionOffset;
		const Vector3 rotationOffsetDegrees = _currentRotationOffsetDegrees + _randomRotationOffsetDegrees;
		const Vector3 rotationOffsetRadians{ Math::DegreesToRadians(rotationOffsetDegrees.x), Math::DegreesToRadians(rotationOffsetDegrees.y), Math::DegreesToRadians(rotationOffsetDegrees.z) };
		const Quaternion rotationOffset = Quaternion::CreateFromYawPitchRoll(rotationOffsetRadians.y, rotationOffsetRadians.x, rotationOffsetRadians.z);
		const Matrix relativeTransform = Matrix::CreateScale(relativeScale) * Matrix::CreateFromQuaternion(rotationOffset * relativeRotation) * Matrix::CreateTranslation(relativePosition);

		if (_attachmentMode == EffectAttachmentMode::World)
		{
			GetTransform()->SetWorldMatrix(relativeTransform * _spawnTransform);
			return true;
		}

		GameObject* owner = _owner.Get();
		if (owner == nullptr)
			return false;

		if (_attachmentMode == EffectAttachmentMode::OwnerPosition)
		{
			const Vector3 ownerPosition = Vector3::Transform(Vector3::Zero, owner->GetTransform()->GetWorldMatrix());
			GetTransform()->SetWorldMatrix(relativeTransform * _spawnTransform * Matrix::CreateTranslation(ownerPosition - _ownerSpawnPosition));
			return true;
		}

		if (_attachmentMode == EffectAttachmentMode::Owner)
		{
			GetTransform()->SetWorldMatrix(relativeTransform * _spawnTransform * owner->GetTransform()->GetWorldMatrix());
			return true;
		}

		if (_attachmentMode == EffectAttachmentMode::SocketPosition)
		{
			if (_socketComponent == nullptr)
				_socketComponent = owner->GetComponent<SocketComponent>();
			GM_ASSERT_RETURN_VAL(_socketComponent && _socketComponent->HasSocket(_socketName), false, "Effect를 부착할 Socket을 찾을 수 없습니다. socket=%ls", _socketName.c_str());
			const Vector3 socketPosition = Vector3::Transform(Vector3::Zero, _socketComponent->GetSocketAnchorWorldMatrix(_socketName));
			GetTransform()->SetWorldMatrix(relativeTransform * _spawnTransform * Matrix::CreateTranslation(socketPosition - _ownerSpawnPosition));
			return true;
		}

		GM_ASSERT_RETURN_VAL(_attachmentMode == EffectAttachmentMode::Socket, false, "지원하지 않는 Effect Attachment Mode입니다.");
		if (_socketComponent == nullptr)
			_socketComponent = owner->GetComponent<SocketComponent>();
		GM_ASSERT_RETURN_VAL(_socketComponent && _socketComponent->HasSocket(_socketName), false, "Effect를 부착할 Socket을 찾을 수 없습니다. socket=%ls", _socketName.c_str());
		GetTransform()->SetWorldMatrix(relativeTransform * _spawnTransform * _socketComponent->GetSocketWorldMatrix(_socketName));
		return true;
	}

	void EffectRuntimeObject::UpdateElapsedTime(float deltaTime)
	{
		const float safeDeltaTime = std::max(0.f, deltaTime);
		_elapsedSeconds += safeDeltaTime;
		if (_isActive)
			_activeElapsedSeconds += safeDeltaTime;

		const BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		if (beatSystem.HasPlaybackTime() == false)
		{
			_previousBeat.reset();
			return;
		}

		const float currentBeat = beatSystem.GetCurrentBeat();
		if (_previousBeat.has_value())
		{
			const float beatDelta = std::max(0.f, currentBeat - _previousBeat.value());
			_elapsedBeats += beatDelta;
			if (_isActive)
				_activeElapsedBeats += beatDelta;
		}

		_previousBeat = currentBeat;
	}

	void EffectRuntimeObject::UpdatePresentation(float deltaTime)
	{
		const float activeRatio = GetActiveRatio();
		UpdateSpriteSheetFrame();
		if (_opacityInterpolationSpeed > 0.f)
		{
			const float interpolationRatio = 1.f - std::exp(-_opacityInterpolationSpeed * std::max(0.f, deltaTime));
			_currentOpacity = std::lerp(_currentOpacity, _endOpacity, interpolationRatio);
		}
		else
		{
			_currentOpacity = std::lerp(_startOpacity, _endOpacity, activeRatio);
		}
		SetOpacity(_currentOpacity);
		if (_spriteComponent)
		{
			if (_animationType == EffectAnimationType::RotationCircle || _animationType == EffectAnimationType::ContinuousRotationCircle)
			{
				_spriteComponent->SetFillRatio(1.f);
				_spriteComponent->SetRadialFill(_radialCenter, _radialStartAngle, CalculateRotationCircleAngle());
			}
			else
			{
				_spriteComponent->SetFillRatio(std::lerp(_startFillRatio, _endFillRatio, activeRatio));
			}
		}
		if (_hasDissolve)
			SetDissolveThreshold(std::lerp(_dissolveStartThreshold, _dissolveEndThreshold, activeRatio));
		if (_emissiveDuration.value > 0.f && _emissiveDuration.HasElapsed(_activeElapsedSeconds, _activeElapsedBeats))
		{
			SetEmissiveIntensity(0.f);
			_emissiveDuration.value = 0.f;
		}
	}

	void EffectRuntimeObject::UpdateSpriteSheetFrame()
	{
		if (_spriteComponent == nullptr || _spriteSheetFrameCount <= 1)
			return;

		const float elapsed = _spriteSheetFrameDuration.unit == EffectTimeUnit::Second ? _activeElapsedSeconds : _activeElapsedBeats;
		const uint32 relativeFrame = std::min(static_cast<uint32>(elapsed / _spriteSheetFrameDuration.value), _spriteSheetFrameCount - 1);
		const uint32 frame = _spriteSheetStartFrame + relativeFrame;
		if (frame == _currentSpriteSheetFrame)
			return;

		const std::shared_ptr<Texture> texture = _spriteComponent->GetTexture();
		if (texture == nullptr)
			return;

		const float frameWidth = static_cast<float>(texture->GetWidth()) / _spriteSheetColumns;
		const float frameHeight = static_cast<float>(texture->GetHeight()) / _spriteSheetRows;
		const uint32 column = frame % _spriteSheetColumns;
		const uint32 row = frame / _spriteSheetColumns;
		_spriteComponent->SetSourceRect(Rect{ frameWidth * column, frameHeight * row, frameWidth, frameHeight });
		_currentSpriteSheetFrame = frame;
	}

	float EffectRuntimeObject::CalculateRotationCircleAngle() const
	{
		const float accumulatedAngle = _activeElapsedBeats * _radialSweepAngle * 3.f * _animationSpeed;
		if (_animationType == EffectAnimationType::ContinuousRotationCircle)
			return accumulatedAngle;
		const float currentAngle = accumulatedAngle > _radialSweepAngle ? _radialSweepAngle + accumulatedAngle * 0.25f : accumulatedAngle;
		return std::min(currentAngle, _radialSweepAngle * 1.5f);
	}

	void EffectRuntimeObject::OverrideOpacity(float opacity)
	{
		_currentOpacity = std::clamp(opacity, 0.f, 1.f);
		_startOpacity = _currentOpacity;
		_endOpacity = _currentOpacity;
		SetOpacity(_currentOpacity);
	}

	void EffectRuntimeObject::SetOpacity(float opacity)
	{
		const float clampedOpacity = std::clamp(opacity, 0.f, 1.f);
		if (_spriteComponent)
		{
			_spriteComponent->SetOpacity(clampedOpacity);
			return;
		}

		if (_staticMeshComponent == nullptr)
			return;

		for (uint32 slotIndex = 0; slotIndex < _staticMeshComponent->GetStaticMesh()->GetMaterialSlotCount(); ++slotIndex)
		{
			Material* material = _staticMeshComponent->GetMaterial(slotIndex);
			if (material == nullptr)
				continue;
			Color colorMultiplier = material->GetColorData().colorMultiplier;
			colorMultiplier.w = clampedOpacity;
			material->SetColorMultiplier(colorMultiplier);
		}
	}

	void EffectRuntimeObject::SetEmissiveIntensity(float intensity)
	{
		_effectMaterialConstant.emissiveIntensity = intensity;
		if (_spriteComponent)
		{
			_spriteComponent->SetEmissive(_effectMaterialConstant.emissiveColor, intensity);
			return;
		}

		if (_staticMeshComponent == nullptr)
			return;

		for (uint32 slotIndex = 0; slotIndex < _staticMeshComponent->GetStaticMesh()->GetMaterialSlotCount(); ++slotIndex)
		{
			Material* material = _staticMeshComponent->GetMaterial(slotIndex);
			if (material)
				material->SetConstantData(ShaderStage::Pixel, EffectMaterialConstantSlot, _effectMaterialConstant);
		}
	}

	void EffectRuntimeObject::SetDissolveThreshold(float threshold)
	{
		if (_spriteComponent)
		{
			_spriteComponent->SetDissolveThreshold(threshold);
			return;
		}

		if (_staticMeshComponent == nullptr)
			return;

		_effectMaterialConstant.dissolveThreshold = threshold;
		for (uint32 slotIndex = 0; slotIndex < _staticMeshComponent->GetStaticMesh()->GetMaterialSlotCount(); ++slotIndex)
		{
			Material* material = _staticMeshComponent->GetMaterial(slotIndex);
			if (material)
				material->SetConstantData(ShaderStage::Pixel, EffectMaterialConstantSlot, _effectMaterialConstant);
		}
	}

	float EffectRuntimeObject::GetActiveRatio() const
	{
		if (_isActive == false)
			return 0.f;

		const float elapsed = _lifetime.unit == EffectTimeUnit::Second ? _activeElapsedSeconds : _activeElapsedBeats;
		return _lifetime.value > 0.f ? std::clamp(elapsed / _lifetime.value, 0.f, 1.f) : 1.f;
	}

	void EffectRuntimeObject::Activate()
	{
		SetRender(true);
		_activeElapsedSeconds = 0.f;
		_activeElapsedBeats = 0.f;
		_isActive = true;
		UpdatePresentation(0.f);
	}
}
