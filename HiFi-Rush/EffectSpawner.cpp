#include "EffectSpawner.h"

#include "BuiltinGraphicsResources.h"
#include "EffectPresetData.h"
#include "EffectPresets.h"
#include "EffectInstance.h"
#include "EffectRuntimeObject.h"
#include "GameObject.h"
#include "Resources.h"
#include "Scene.h"
#include "Shader.h"
#include "SocketComponent.h"
#include "StaticMesh.h"
#include "Texture.h"
#include "TransformComponent.h"

#include <vector>

namespace gm
{
	EffectSpawner::EffectSpawner(Resources& resources, const EffectPresets& effectPresets)
		: _resources(resources), _effectPresets(effectPresets)
	{
	}

	bool EffectSpawner::SpawnAtWorld(Scene& scene, const std::wstring& effectId, const Matrix& world) const
	{
		return Spawn(scene, effectId, EffectAttachmentMode::World, world, nullptr, {});
	}

	bool EffectSpawner::SpawnAtWorld(Scene& scene, const std::wstring& effectId, const Matrix& world, EffectInstance& outInstance) const
	{
		return Spawn(scene, effectId, EffectAttachmentMode::World, world, nullptr, {}, &outInstance);
	}

	bool EffectSpawner::SpawnAttachedToOwner(Scene& scene, const std::wstring& effectId, GameObject& owner, const Matrix& localTransform) const
	{
		return Spawn(scene, effectId, EffectAttachmentMode::Owner, localTransform, &owner, {});
	}

	bool EffectSpawner::SpawnAttachedToSocket(Scene& scene, const std::wstring& effectId, GameObject& owner, const std::wstring& socketName, const Matrix& localTransform) const
	{
		return Spawn(scene, effectId, EffectAttachmentMode::Socket, localTransform, &owner, socketName);
	}

	bool EffectSpawner::Spawn(Scene& scene, const std::wstring& effectId, EffectAttachmentMode attachmentMode, const Matrix& spawnTransform, GameObject* owner, const std::wstring& socketName, EffectInstance* outInstance) const
	{
		const EffectPresetData* preset = _effectPresets.Find(effectId);
		GM_ASSERT_RETURN_VAL(preset, false, "Effect Preset을 찾을 수 없습니다. id=%ls", effectId.c_str());
		GM_ASSERT_RETURN_VAL(preset->tracks.empty() == false, false, "Effect Preset에 Track이 없습니다. id=%ls", preset->id.c_str());
		GM_ASSERT_RETURN_VAL(ValidateAttachment(attachmentMode, owner, socketName), false, "Effect 부착 정보가 유효하지 않습니다. preset=%ls", preset->id.c_str());

		std::vector<EffectRuntimeObjectDesc> runtimeObjectDescs(preset->tracks.size());
		for (size_t index = 0; index < preset->tracks.size(); ++index)
		{
			if (CreateRuntimeObjectDesc(preset->tracks[index], attachmentMode, spawnTransform, owner, socketName, runtimeObjectDescs[index]) == false)
				return false;
		}

		std::vector<WeakGameObjectPtr> trackObjects;
		trackObjects.reserve(runtimeObjectDescs.size());
		for (const EffectRuntimeObjectDesc& desc : runtimeObjectDescs)
		{
			EffectRuntimeObject* runtimeObject = scene.SpawnGameObject<EffectRuntimeObject>(desc);
			if (runtimeObject == nullptr)
			{
				for (WeakGameObjectPtr& trackObject : trackObjects)
				{
					if (trackObject)
						trackObject->Destroy();
				}
				return false;
			}
			trackObjects.push_back(runtimeObject->GetWeakPtr());
		}
		if (outInstance)
			outInstance->SetTrackObjects(std::move(trackObjects));
		return true;
	}

	bool EffectSpawner::ValidateAttachment(EffectAttachmentMode attachmentMode, GameObject* owner, const std::wstring& socketName) const
	{
		GM_ASSERT_RETURN_VAL(attachmentMode < EffectAttachmentMode::Count, false, "지원하지 않는 Effect Attachment Mode입니다.");
		if (attachmentMode == EffectAttachmentMode::World)
			return true;

		GM_ASSERT_RETURN_VAL(owner && owner->IsPendingDestroy() == false, false, "Effect를 부착할 Owner가 유효하지 않습니다.");
		if (attachmentMode == EffectAttachmentMode::Owner)
			return true;

		GM_ASSERT_RETURN_VAL(socketName.empty() == false, false, "Effect를 부착할 Socket 이름이 비어 있습니다.");
		const SocketComponent* socketComponent = owner->GetComponent<SocketComponent>();
		GM_ASSERT_RETURN_VAL(socketComponent && socketComponent->HasSocket(socketName), false, "Effect를 부착할 Socket을 찾을 수 없습니다. socket=%ls", socketName.c_str());
		return true;
	}

	bool EffectSpawner::CreateRuntimeObjectDesc(const EffectTrackData& track, EffectAttachmentMode attachmentMode, const Matrix& spawnTransform, GameObject* owner, const std::wstring& socketName, _Out_ EffectRuntimeObjectDesc& outDesc) const
	{
		GM_ASSERT_RETURN_VAL(track.visualType < EffectVisualType::Count, false, "지원하지 않는 Effect Visual Type입니다.");
		GM_ASSERT_RETURN_VAL(track.attachmentMode < EffectTrackAttachmentMode::Count, false, "지원하지 않는 Effect Track Attachment Mode입니다.");
		GM_ASSERT_RETURN_VAL(track.resourceKey.empty() == false, false, "Effect Track Resource Key가 비어 있습니다.");
		GM_ASSERT_RETURN_VAL(track.delay.unit < EffectTimeUnit::Count && track.delay.value >= 0.f, false, "Effect Track Delay가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(track.lifetime.unit < EffectTimeUnit::Count && track.lifetime.value > 0.f, false, "Effect Track Lifetime이 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(track.scaleInterpolationDelay.unit < EffectTimeUnit::Count && track.scaleInterpolationDelay.value >= 0.f, false, "Effect Track Scale Interpolation Delay가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(track.opacity >= 0.f && track.opacity <= 1.f, false, "Effect Track Opacity는 0과 1 사이여야 합니다.");
		GM_ASSERT_RETURN_VAL(track.endOpacity >= 0.f && track.endOpacity <= 1.f, false, "Effect Track End Opacity는 0과 1 사이여야 합니다.");
		GM_ASSERT_RETURN_VAL(track.opacityInterpolationSpeed >= 0.f, false, "Effect Track Opacity Interpolation Speed는 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(track.startFillRatio >= 0.f && track.startFillRatio <= 1.f, false, "Effect Track Start Fill Ratio는 0과 1 사이여야 합니다.");
		GM_ASSERT_RETURN_VAL(track.endFillRatio >= 0.f && track.endFillRatio <= 1.f, false, "Effect Track End Fill Ratio는 0과 1 사이여야 합니다.");
		GM_ASSERT_RETURN_VAL(track.radialSweepAngle >= 0.f, false, "Effect Track Radial Sweep Angle은 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(track.positionInterpolationSpeed >= 0.f && track.rotationInterpolationSpeed >= 0.f && track.scaleInterpolationSpeed >= 0.f && track.angularVelocityInterpolationSpeed >= 0.f, false, "Effect Track Transform Interpolation Speed는 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(track.animationType < EffectAnimationType::Count && track.animationSpeed >= 0.f, false, "Effect Track Animation 설정이 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(track.emissiveIntensity >= 0.f, false, "Effect Track Emissive Intensity는 0 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(track.emissiveDuration.unit < EffectTimeUnit::Count && track.emissiveDuration.value >= 0.f, false, "Effect Track Emissive Duration이 유효하지 않습니다.");
		const uint64 spriteSheetCellCount = static_cast<uint64>(track.spriteSheetColumns) * track.spriteSheetRows;
		GM_ASSERT_RETURN_VAL(track.spriteSheetColumns > 0 && track.spriteSheetRows > 0 && track.spriteSheetFrameCount > 0, false, "Effect Sprite Sheet의 열, 행과 Frame 수는 1 이상이어야 합니다.");
		GM_ASSERT_RETURN_VAL(static_cast<uint64>(track.spriteSheetStartFrame) + track.spriteSheetFrameCount <= spriteSheetCellCount, false, "Effect Sprite Sheet Frame 범위가 유효하지 않습니다.");
		GM_ASSERT_RETURN_VAL(track.spriteSheetFrameDuration.unit < EffectTimeUnit::Count && (track.spriteSheetFrameCount == 1 || track.spriteSheetFrameDuration.value > 0.f), false, "Effect Sprite Sheet Frame Duration이 유효하지 않습니다.");

		outDesc = {};
		outDesc.track = track;
		outDesc.attachmentMode = attachmentMode;
		outDesc.spawnTransform = spawnTransform;
		outDesc.owner = owner ? owner->GetWeakPtr() : WeakGameObjectPtr{};
		outDesc.socketName = socketName;
		if (attachmentMode == EffectAttachmentMode::Owner && track.attachmentMode != EffectTrackAttachmentMode::Inherit)
		{
			const Matrix ownerWorld = owner->GetTransform()->GetWorldMatrix();
			outDesc.spawnTransform = spawnTransform * ownerWorld;
			if (track.attachmentMode == EffectTrackAttachmentMode::WorldAtSpawn)
				outDesc.attachmentMode = EffectAttachmentMode::World;
			else if (track.attachmentMode == EffectTrackAttachmentMode::FollowOwnerPosition)
			{
				outDesc.attachmentMode = EffectAttachmentMode::OwnerPosition;
				outDesc.ownerSpawnPosition = Vector3::Transform(Vector3::Zero, ownerWorld);
			}
			else
			{
				GM_ASSERT_RETURN_VAL(track.attachmentMode == EffectTrackAttachmentMode::FollowSocketPosition && track.attachmentSocketName.empty() == false, false, "Effect Track Socket Attachment 설정이 유효하지 않습니다. socket=%ls", track.attachmentSocketName.c_str());
				const SocketComponent* socketComponent = owner->GetComponent<SocketComponent>();
				GM_ASSERT_RETURN_VAL(socketComponent && socketComponent->HasSocket(track.attachmentSocketName), false, "Effect Track을 부착할 Socket을 찾을 수 없습니다. socket=%ls", track.attachmentSocketName.c_str());
				outDesc.attachmentMode = EffectAttachmentMode::SocketPosition;
				outDesc.ownerSpawnPosition = Vector3::Transform(Vector3::Zero, ownerWorld);
				outDesc.socketName = track.attachmentSocketName;
			}
		}
		if (track.visualType == EffectVisualType::Sprite)
		{
			outDesc.spriteTexture = _resources.Find<Texture>(track.resourceKey);
			GM_ASSERT_RETURN_VAL(outDesc.spriteTexture, false, "Effect Sprite Texture가 로드되지 않았습니다. key=%ls", track.resourceKey.c_str());
		}
		else
		{
			outDesc.staticMesh = _resources.Find<StaticMesh>(track.resourceKey);
			outDesc.effectMeshShader = _resources.Find<Shader>(BuiltinResourceKey::EffectMeshPS);
			GM_ASSERT_RETURN_VAL(outDesc.staticMesh && outDesc.effectMeshShader, false, "Effect StaticMesh 또는 Effect Mesh Shader가 로드되지 않았습니다. key=%ls", track.resourceKey.c_str());
		}
		if (track.baseColorTextureKey.empty() == false)
		{
			outDesc.baseColorTexture = _resources.Find<Texture>(track.baseColorTextureKey);
			GM_ASSERT_RETURN_VAL(outDesc.baseColorTexture, false, "Effect BaseColor Texture가 로드되지 않았습니다. key=%ls", track.baseColorTextureKey.c_str());
		}

		if (track.dissolveTextureKey.empty() == false)
		{
			outDesc.dissolveTexture = _resources.Find<Texture>(track.dissolveTextureKey);
			GM_ASSERT_RETURN_VAL(outDesc.dissolveTexture, false, "Effect Dissolve Texture가 로드되지 않았습니다. key=%ls", track.dissolveTextureKey.c_str());
		}
		return true;
	}
}
