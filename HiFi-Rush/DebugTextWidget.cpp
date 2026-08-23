#include "DebugTextWidget.h"

#if GM_ENABLE_DEBUG_TOOLS

#include "Application.h"
#include "BeatSystem.h"
#include "BloomRenderPass.h"
#include "CameraManager.h"
#include "ChiEffectComponent.h"
#include "EnvironmentModelDebugLabelComponent.h"
#include "EffectPresets.h"
#include "GameObject.h"
#include "GMLog.h"
#include "HiFiRushAudio.h"
#include "HiFiRushStatics.h"
#include "IDebugRenderer.h"
#include "Input.h"
#include "Material.h"
#include "NavMeshSystem.h"
#include "PhysicsSystem.h"
#include "QamilStateMachineComponent.h"
#include "ReverbComponent.h"
#include "Renderer.h"
#include "RhythmInputJudge.h"
#include "Scene.h"
#include "SceneManager.h"
#include "ScreenSpaceOutlinePass.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "SSAORenderPass.h"
#include "SpotLightComponent.h"
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "TextBlock.h"
#include "UIManager.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

namespace gm
{
	namespace
	{
		constexpr uint32 BeatProgressBarWidth = 20;
		constexpr float BeatPulseDuration = 0.1f;
		constexpr float MaterialHighlightDuration = 0.4f;
		constexpr float AudioOffsetStep = 0.01f;
		constexpr float AudioOffsetCoarseStep = 0.05f;
		constexpr float AudioOffsetSettledThreshold = 0.005f;
		constexpr wchar_t WeakAttackRotationCircleResourceKey[] = L"FX_RotationCircle_WeakAttack";
		constexpr wchar_t WeakAttackBeatHitEffectId[] = L"Chi.BeatHit.ANIM_ATTACK_WEAK3";
		constexpr wchar_t WeakAttackBeatHitResourceKey[] = L"WeakAttack_BeatHit";
		constexpr wchar_t WeakAttackBeatHitFloorResourceKey[] = L"FX_WeakAttackBeatHit_Floor";
		constexpr wchar_t StrongAttackFirstEffectId[] = L"Chi.Attack.ANIM_ATTACK_STRONG0_1";
		constexpr wchar_t StrongAttackSecondEffectId[] = L"Chi.Attack.ANIM_ATTACK_STRONG1";
		constexpr wchar_t StrongAttackThirdEffectId[] = L"Chi.Attack.ANIM_ATTACK_STRONG2";
		constexpr wchar_t StrongAttackRotationCircleResourceKey[] = L"FX_RotationCircle_StrongAttack";
		constexpr wchar_t StrongAttackUpAxisResourceKey[] = L"FX_UpAxis_StrongAttack";
		constexpr wchar_t SwordChargeResourceKey[] = L"FX_SwordTwinkle";
		constexpr wchar_t SwordSlashResourceKey[] = L"FX_RotationCircle_SwordSlash";
		constexpr wchar_t StumpEffectId[] = L"Chi.Stump";
		constexpr wchar_t StumpFloorResourceKey[] = L"FX_StumpFloor_Bloom";
		constexpr wchar_t StumpCloudResourceKey[] = L"Cloud";
		constexpr wchar_t StumpSphereResourceKey[] = L"WhiteSphere";
		constexpr wchar_t QamilStumpFloorEffectId[] = L"Qamil.Stump.Floor";
		constexpr wchar_t QamilStumpFloorResourceKey[] = L"FX_StumpFloor";
		constexpr wchar_t HibikiCloudEffectId[] = L"Chi.Hibiki.Cloud";
		constexpr wchar_t HibikiCloudResourceKey[] = L"Cloud";
		constexpr float EffectEmissiveAdjustmentStep = 0.05f;
		constexpr float EffectEmissiveCoarseAdjustmentStep = 0.5f;
		enum class EffectSetting : uint32
		{
			WeakAttackEmissive,
			WeakAttackBeatHitEmissive,
			WeakAttackBeatHitFloorEmissive,
			StrongAttackEmissive,
			SwordChargeEmissive,
			SwordSlashEmissive,
			StumpFloorEmissive,
			StumpCloudEmissive,
			StumpSphereEmissive,
			QamilStumpFloorEmissive,
			HibikiCloudEmissive,
			AfterImageEmissive,
			AfterImageOpacity,
			AfterImageColorBlendRatio,
			AfterImageFirstColorR,
			AfterImageFirstColorG,
			AfterImageFirstColorB,
			AfterImageSecondColorR,
			AfterImageSecondColorG,
			AfterImageSecondColorB,
			AfterImageThirdColorR,
			AfterImageThirdColorG,
			AfterImageThirdColorB,
			Count
		};
		struct EffectTrackDebugTarget
		{
			const wchar_t* effectId;
			const wchar_t* resourceKey;
		};
		constexpr std::array StrongAttackMainEffectTargets
		{
			EffectTrackDebugTarget{ StrongAttackFirstEffectId, StrongAttackRotationCircleResourceKey },
			EffectTrackDebugTarget{ StrongAttackSecondEffectId, StrongAttackUpAxisResourceKey },
			EffectTrackDebugTarget{ StrongAttackThirdEffectId, StrongAttackRotationCircleResourceKey }
		};

		void AdjustEffectResourceEmissive(const wchar_t* resourceKey, const char* displayName, float amount)
		{
			EffectPresets& effectPresets = HiFiRushStatics::GetMutableEffectPresets();
			const std::optional<float> currentIntensity = effectPresets.FindEmissiveIntensity(resourceKey);
			if (currentIntensity.has_value() == false)
			{
				GM_LOG("[Effect Debug] %s Track을 찾을 수 없습니다.", displayName);
				return;
			}
			const float intensity = std::max(0.f, currentIntensity.value() + amount);
			effectPresets.SetEmissiveIntensity(resourceKey, intensity);
			GM_LOG("[Effect Debug] %s Emissive Intensity: %.2f", displayName, intensity);
		}

		std::optional<float> FindStrongAttackMainEmissiveIntensity()
		{
			const EffectTrackDebugTarget& target = StrongAttackMainEffectTargets.front();
			return HiFiRushStatics::GetEffectPresets().FindEmissiveIntensity(target.effectId, target.resourceKey);
		}

		void AdjustStrongAttackMainEmissive(float amount)
		{
			EffectPresets& effectPresets = HiFiRushStatics::GetMutableEffectPresets();
			const std::optional<float> currentIntensity = FindStrongAttackMainEmissiveIntensity();
			if (currentIntensity.has_value() == false)
			{
				GM_LOG("[Effect Debug] Strong Attack 1~3 Main Track을 찾을 수 없습니다.");
				return;
			}
			const float intensity = std::max(0.f, currentIntensity.value() + amount);
			for (const EffectTrackDebugTarget& target : StrongAttackMainEffectTargets)
				effectPresets.SetEmissiveIntensity(target.effectId, target.resourceKey, intensity);
			GM_LOG("[Effect Debug] Strong Attack 1~3 Main Emissive Intensity: %.2f", intensity);
		}

		void AdjustEffectTrackEmissive(const EffectTrackDebugTarget& target, const char* displayName, float amount)
		{
			EffectPresets& effectPresets = HiFiRushStatics::GetMutableEffectPresets();
			const std::optional<float> currentIntensity = effectPresets.FindEmissiveIntensity(target.effectId, target.resourceKey);
			if (currentIntensity.has_value() == false)
			{
				GM_LOG("[Effect Debug] %s Track을 찾을 수 없습니다.", displayName);
				return;
			}
			const float intensity = std::max(0.f, currentIntensity.value() + amount);
			effectPresets.SetEmissiveIntensity(target.effectId, target.resourceKey, intensity);
			GM_LOG("[Effect Debug] %s Emissive Intensity: %.2f", displayName, intensity);
		}

		const wchar_t* GetRhythmJudgeGradeName(RhythmJudgeGrade grade)
		{
			switch (grade)
			{
			case RhythmJudgeGrade::Perfect: return L"Perfect";
			case RhythmJudgeGrade::Good: return L"Good";
			case RhythmJudgeGrade::OffBeat: return L"OffBeat";
			default: return L"Unknown";
			}
		}

		const wchar_t* GetAudioOffsetRecommendation(float errorSeconds)
		{
			if (std::abs(errorSeconds) <= AudioOffsetSettledThreshold)
				return L"현재 Offset 유지";
			return errorSeconds < 0.f ? L"] 키로 Offset을 미세요" : L"[ 키로 Offset을 당기세요";
		}

		enum class AtmosphereSetting : uint32
		{
			SunlightPreset,
			AmbientIntensity,
			AmbientColorR,
			AmbientColorG,
			AmbientColorB,
			SpotLightIntensity,
			FogEnabled,
			FogColorR,
			FogColorG,
			FogColorB,
			FogStartDistance,
			FogEndDistance,
			FogDensity,
			Count
		};

		enum class PostProcessSetting : uint32
		{
			Exposure,
			BloomThreshold,
			BloomSoftKnee,
			BloomIntensity,
			SSAORadius,
			SSAOFadeStart,
			SSAOFadeEnd,
			SSAOSurfaceEpsilon,
			SSAOStrength,
			OutlineEnabled,
			OutlineDepthThreshold,
			OutlineNormalThreshold,
			OutlineThickness,
			OutlineColorR,
			OutlineColorG,
			OutlineColorB,
			OutlineOpacity,
			FXAAEnabled,
			Count
		};

		enum class ShadowSetting : uint32
		{
			CascadeColors,
			Resolution,
			CascadeCount,
			PCFRadius,
			Strength,
			SplitLambda,
			ColorR,
			ColorG,
			ColorB,
			DepthBias,
			NormalBias,
			MaxDistance,
			Count
		};

		constexpr uint32 AtmosphereSettingCount = static_cast<uint32>(AtmosphereSetting::Count);
		constexpr uint32 EffectSettingCount = static_cast<uint32>(EffectSetting::Count);
		constexpr uint32 PostProcessSettingCount = static_cast<uint32>(PostProcessSetting::Count);
		constexpr uint32 ShadowSettingCount = static_cast<uint32>(ShadowSetting::Count);
		constexpr std::array<uint32, 3> ShadowResolutionPresets = { 512, 1024, 2048 };

		ChiEffectComponent* FindChiEffectComponent(Scene* scene)
		{
			ChiEffectComponent* result = nullptr;
			if (scene == nullptr)
				return result;
			scene->ForEachGameObject([&result](GameObject& gameObject)
			{
				if (result == nullptr)
					result = gameObject.GetComponent<ChiEffectComponent>();
			});
			return result;
		}

		ReverbComponent* FindPlayerReverbComponent(Scene* scene)
		{
			ReverbComponent* result = nullptr;
			if (scene == nullptr)
				return result;
			scene->ForEachGameObject([&result](GameObject& gameObject)
			{
				if (result == nullptr)
					result = gameObject.GetComponent<ReverbComponent>();
			});
			return result;
		}

		void FillPlayerReverb(Scene* scene)
		{
			ReverbComponent* reverbComponent = FindPlayerReverbComponent(scene);
			if (reverbComponent == nullptr)
			{
				GM_LOG("[Reverb Debug] ReverbComponent를 찾을 수 없습니다.");
				return;
			}
			reverbComponent->SetReverb(reverbComponent->GetMaxReverb());
			GM_LOG("[Reverb Debug] Reverb: %.2f / %.2f", reverbComponent->GetReverb(), reverbComponent->GetMaxReverb());
		}

		void AdjustEffectSetting(Scene* scene, EffectSetting setting, float direction, bool coarseAdjustment)
		{
			const float emissiveAmount = direction * (coarseAdjustment ? EffectEmissiveCoarseAdjustmentStep : EffectEmissiveAdjustmentStep);
			switch (setting)
			{
			case EffectSetting::WeakAttackEmissive: AdjustEffectResourceEmissive(WeakAttackRotationCircleResourceKey, "Weak Attack Rotation Circle", emissiveAmount); break;
			case EffectSetting::WeakAttackBeatHitEmissive: AdjustEffectTrackEmissive({ WeakAttackBeatHitEffectId, WeakAttackBeatHitResourceKey }, "Weak Attack Beat Hit", emissiveAmount); break;
			case EffectSetting::WeakAttackBeatHitFloorEmissive: AdjustEffectTrackEmissive({ WeakAttackBeatHitEffectId, WeakAttackBeatHitFloorResourceKey }, "Weak Attack Beat Hit Floor", emissiveAmount); break;
			case EffectSetting::StrongAttackEmissive: AdjustStrongAttackMainEmissive(emissiveAmount); break;
			case EffectSetting::SwordChargeEmissive: AdjustEffectResourceEmissive(SwordChargeResourceKey, "Sword Charge", emissiveAmount); break;
			case EffectSetting::SwordSlashEmissive: AdjustEffectResourceEmissive(SwordSlashResourceKey, "Sword Slash", emissiveAmount); break;
			case EffectSetting::StumpFloorEmissive: AdjustEffectTrackEmissive({ StumpEffectId, StumpFloorResourceKey }, "Stump Floor", emissiveAmount); break;
			case EffectSetting::StumpCloudEmissive: AdjustEffectTrackEmissive({ StumpEffectId, StumpCloudResourceKey }, "Stump Cloud", emissiveAmount); break;
			case EffectSetting::StumpSphereEmissive: AdjustEffectTrackEmissive({ StumpEffectId, StumpSphereResourceKey }, "Stump Sphere", emissiveAmount); break;
			case EffectSetting::QamilStumpFloorEmissive: AdjustEffectTrackEmissive({ QamilStumpFloorEffectId, QamilStumpFloorResourceKey }, "Qamil Stump Floor", emissiveAmount); break;
			case EffectSetting::HibikiCloudEmissive: AdjustEffectTrackEmissive({ HibikiCloudEffectId, HibikiCloudResourceKey }, "Hibiki Cloud", emissiveAmount); break;
			case EffectSetting::AfterImageEmissive:
			{
				ChiEffectComponent* effectComponent = FindChiEffectComponent(scene);
				if (effectComponent == nullptr)
				{
					GM_LOG("[Effect Debug] ChiEffectComponent를 찾을 수 없습니다.");
					break;
				}
				effectComponent->SetAfterImageEmissiveIntensity(effectComponent->GetAfterImageEmissiveIntensity() + emissiveAmount);
				GM_LOG("[Effect Debug] After Image Emissive Intensity: %.2f", effectComponent->GetAfterImageEmissiveIntensity());
				break;
			}
			case EffectSetting::AfterImageOpacity:
			{
				ChiEffectComponent* effectComponent = FindChiEffectComponent(scene);
				if (effectComponent == nullptr)
					break;
				effectComponent->SetAfterImageOpacity(effectComponent->GetAfterImageOpacity() + direction * 0.02f * (coarseAdjustment ? 5.f : 1.f));
				GM_LOG("[Effect Debug] After Image Opacity: %.2f", effectComponent->GetAfterImageOpacity());
				break;
			}
			case EffectSetting::AfterImageColorBlendRatio:
			{
				ChiEffectComponent* effectComponent = FindChiEffectComponent(scene);
				if (effectComponent == nullptr)
					break;
				effectComponent->SetAfterImageColorBlendRatio(effectComponent->GetAfterImageColorBlendRatio() + direction * 0.02f * (coarseAdjustment ? 5.f : 1.f));
				GM_LOG("[Effect Debug] After Image Color Blend Ratio: %.2f", effectComponent->GetAfterImageColorBlendRatio());
				break;
			}
			default:
			{
				ChiEffectComponent* effectComponent = FindChiEffectComponent(scene);
				if (effectComponent == nullptr)
					break;
				const uint32 settingOffset = static_cast<uint32>(setting) - static_cast<uint32>(EffectSetting::AfterImageFirstColorR);
				const uint32 afterImageIndex = settingOffset / 3;
				const uint32 colorChannel = settingOffset % 3;
				Color color = effectComponent->GetAfterImageColor(afterImageIndex);
				float* channel = colorChannel == 0 ? &color.x : colorChannel == 1 ? &color.y : &color.z;
				*channel = std::clamp(*channel + direction * 0.02f * (coarseAdjustment ? 5.f : 1.f), 0.f, 1.f);
				effectComponent->SetAfterImageColor(afterImageIndex, color);
				GM_LOG("[Effect Debug] After Image %u Color: (%.2f, %.2f, %.2f)", afterImageIndex + 1, color.x, color.y, color.z);
				break;
			}
			}
		}

		BoundingVolume GetMeshWorldBounds(GameObject& gameObject)
		{
			if (StaticMeshComponent* staticMesh = gameObject.GetComponent<StaticMeshComponent>())
				return staticMesh->GetWorldBounds();
			if (SkeletalMeshComponent* skeletalMesh = gameObject.GetComponent<SkeletalMeshComponent>())
				return skeletalMesh->GetWorldBounds();
			return {};
		}

		uint32 GetMaterialSlotCount(GameObject& gameObject)
		{
			if (StaticMeshComponent* staticMesh = gameObject.GetComponent<StaticMeshComponent>())
				return staticMesh->GetStaticMesh() ? staticMesh->GetStaticMesh()->GetMaterialSlotCount() : 0;
			if (SkeletalMeshComponent* skeletalMesh = gameObject.GetComponent<SkeletalMeshComponent>())
				return skeletalMesh->GetSkeletalMesh() ? skeletalMesh->GetSkeletalMesh()->GetMaterialSlotCount() : 0;
			return 0;
		}

		Material* GetMaterial(GameObject& gameObject, uint32 materialSlot)
		{
			if (StaticMeshComponent* staticMesh = gameObject.GetComponent<StaticMeshComponent>())
				return staticMesh->GetMaterial(materialSlot);
			if (SkeletalMeshComponent* skeletalMesh = gameObject.GetComponent<SkeletalMeshComponent>())
				return skeletalMesh->GetMaterial(materialSlot);
			return nullptr;
		}

		const MeshMaterialSlot* GetMaterialSlot(GameObject& gameObject, uint32 materialSlot)
		{
			if (StaticMeshComponent* staticMesh = gameObject.GetComponent<StaticMeshComponent>())
				return staticMesh->GetStaticMesh() ? staticMesh->GetStaticMesh()->GetMaterialSlot(materialSlot) : nullptr;
			if (SkeletalMeshComponent* skeletalMesh = gameObject.GetComponent<SkeletalMeshComponent>())
				return skeletalMesh->GetSkeletalMesh() ? skeletalMesh->GetSkeletalMesh()->GetMaterialSlot(materialSlot) : nullptr;
			return nullptr;
		}

		GameObject* FindQamil(Scene& scene)
		{
			GameObject* qamil = nullptr;
			scene.ForEachGameObject([&qamil](GameObject& gameObject)
			{
				if (qamil == nullptr && gameObject.GetComponent<QamilStateMachineComponent>())
					qamil = &gameObject;
			});
			return qamil;
		}

		bool SelectMeshByRay(Scene& scene, const Vector2& screenPosition, GameObject* currentSelection, GameObject*& outGameObject, std::optional<uint32>& outModelIndex, uint32& outMaterialSlotCount)
		{
			const CameraManager* cameraManager = scene.GetCameraManager();
			if (cameraManager == nullptr || cameraManager->GetActiveCamera() == nullptr)
				return false;
			const CameraViewInfo viewInfo = cameraManager->GetViewInfo();
			const float screenWidth = static_cast<float>(APPLICATION.GetWidth());
			const float screenHeight = static_cast<float>(APPLICATION.GetHeight());
			if (screenWidth <= 0.f || screenHeight <= 0.f)
				return false;
			const float ndcX = screenPosition.x / screenWidth * 2.f - 1.f;
			const float ndcY = 1.f - screenPosition.y / screenHeight * 2.f;
			const Matrix inverseViewProjection = (viewInfo.view * viewInfo.projection).Invert();
			const Vector4 nearPosition = Vector4::Transform(Vector4{ ndcX, ndcY, 0.f, 1.f }, inverseViewProjection);
			const Vector4 farPosition = Vector4::Transform(Vector4{ ndcX, ndcY, 1.f, 1.f }, inverseViewProjection);
			const Vector3 rayOrigin = Vector3{ nearPosition.x, nearPosition.y, nearPosition.z } / nearPosition.w;
			const Vector3 rayEnd = Vector3{ farPosition.x, farPosition.y, farPosition.z } / farPosition.w;
			Vector3 rayDirection = rayEnd - rayOrigin;
			rayDirection.Normalize();
			struct MeshRayHit
			{
				GameObject* gameObject = nullptr;
				float distance = 0.f;
			};
			std::vector<MeshRayHit> rayHits;
			scene.ForEachGameObject([&rayOrigin, &rayDirection, &rayHits](GameObject& gameObject)
			{
				if (gameObject.GetComponent<StaticMeshComponent>() == nullptr && gameObject.GetComponent<SkeletalMeshComponent>() == nullptr)
					return;
				const BoundingVolume worldBounds = GetMeshWorldBounds(gameObject);
				float distance = 0.f;
				if (worldBounds.isValid == false || Math::IntersectsRay(worldBounds.box, rayOrigin, rayDirection, distance) == false || distance <= 0.0001f)
					return;
				rayHits.push_back({ &gameObject, distance });
			});
			if (rayHits.empty())
			{
				GM_LOG("[Material Debug] Mouse Ray와 충돌한 Mesh World Bounds가 없습니다.");
				return false;
			}
			std::sort(rayHits.begin(), rayHits.end(), [](const MeshRayHit& lhs, const MeshRayHit& rhs) { return lhs.distance < rhs.distance; });
			size_t selectedHitIndex = 0;
			const auto currentHit = std::find_if(rayHits.begin(), rayHits.end(), [currentSelection](const MeshRayHit& hit) { return hit.gameObject == currentSelection; });
			if (currentHit != rayHits.end())
				selectedHitIndex = (static_cast<size_t>(std::distance(rayHits.begin(), currentHit)) + 1) % rayHits.size();
			GameObject* selectedObject = rayHits[selectedHitIndex].gameObject;
			const float selectedDistance = rayHits[selectedHitIndex].distance;
			const EnvironmentModelDebugLabelComponent* selectedLabel = selectedObject->GetComponent<EnvironmentModelDebugLabelComponent>();
			outModelIndex = selectedLabel ? std::optional<uint32>{ selectedLabel->GetModelIndex() } : std::nullopt;
			outMaterialSlotCount = GetMaterialSlotCount(*selectedObject);
			outGameObject = selectedObject;
			if (outModelIndex)
				GM_LOG("[Material Debug] Environment Model %u 선택, Material Slots: %u, Bounds Distance: %.2f, Ray Hits: %zu", *outModelIndex, outMaterialSlotCount, selectedDistance, rayHits.size());
			else
				GM_LOG("[Material Debug] Mesh 선택, Material Slots: %u, Bounds Distance: %.2f, Ray Hits: %zu", outMaterialSlotCount, selectedDistance, rayHits.size());
			return outMaterialSlotCount > 0;
		}

		void AdjustMaterialEmissiveIntensity(Scene& scene, GameObject& selectedObject, const std::optional<uint32>& modelIndex, uint32 materialSlot, float amount)
		{
			Material* selectedMaterial = GetMaterial(selectedObject, materialSlot);
			if (selectedMaterial == nullptr)
				return;
			const float intensity = std::max(selectedMaterial->GetEmissiveIntensity() + amount, 0.f);
			uint32 adjustedCount = 0;
			if (modelIndex == std::nullopt)
			{
				selectedMaterial->SetEmissiveIntensity(intensity);
				adjustedCount = 1;
			}
			else
			{
				scene.ForEachGameObject([modelIndex, materialSlot, intensity, &adjustedCount](GameObject& gameObject)
				{
					const EnvironmentModelDebugLabelComponent* label = gameObject.GetComponent<EnvironmentModelDebugLabelComponent>();
					if (label == nullptr || label->GetModelIndex() != *modelIndex)
						return;
					Material* material = GetMaterial(gameObject, materialSlot);
					if (material == nullptr)
						return;
					material->SetEmissiveIntensity(intensity);
					++adjustedCount;
				});
			}
			if (modelIndex)
				GM_LOG("[Material Debug] Environment Model %u, Slot %u, Emissive Intensity: %.2f, Objects: %u", *modelIndex, materialSlot, intensity, adjustedCount);
			else
				GM_LOG("[Material Debug] Selected Mesh, Slot %u, Emissive Intensity: %.2f", materialSlot, intensity);
		}

		struct SunlightPresetData
		{
			const wchar_t*	name = L"";
			Color			color = Colors::White;
			float			intensity = 0.f;
		};

		const std::array SunlightPresets
		{
			SunlightPresetData{ L"Neutral Noon", Color{ 1.f, 1.f, 1.f, 1.f }, 0.55f },
			SunlightPresetData{ L"Bright Noon", Color{ 1.f, 0.98f, 0.94f, 1.f }, 0.70f },
			SunlightPresetData{ L"Soft Sunlight", Color{ 1.f, 0.96f, 0.88f, 1.f }, 0.65f },
			SunlightPresetData{ L"Warm Afternoon", Color{ 1.f, 0.92f, 0.82f, 1.f }, 0.50f },
			SunlightPresetData{ L"Golden Hour", Color{ 1.f, 0.82f, 0.62f, 1.f }, 0.55f },
		};

		int32 FindSunlightPreset(const SceneAmbientSettings& settings)
		{
			for (uint32 index = 0; index < SunlightPresets.size(); ++index)
			{
				const SunlightPresetData& preset = SunlightPresets[index];
				if (std::abs(settings.ambientColor.x - preset.color.x) < 0.001f && std::abs(settings.ambientColor.y - preset.color.y) < 0.001f && std::abs(settings.ambientColor.z - preset.color.z) < 0.001f && std::abs(settings.ambientIntensity - preset.intensity) < 0.001f)
					return static_cast<int32>(index);
			}
			return -1;
		}

		const wchar_t* GetSunlightPresetName(const SceneAmbientSettings& settings)
		{
			const int32 presetIndex = FindSunlightPreset(settings);
			return presetIndex >= 0 ? SunlightPresets[presetIndex].name : L"Custom";
		}

		void SelectSunlightPreset(SceneAmbientSettings& settings, float direction)
		{
			int32 presetIndex = FindSunlightPreset(settings);
			if (presetIndex < 0)
				presetIndex = direction > 0.f ? -1 : 0;
			const int32 presetCount = static_cast<int32>(SunlightPresets.size());
			presetIndex = (presetIndex + (direction > 0.f ? 1 : -1) + presetCount) % presetCount;
			const SunlightPresetData& preset = SunlightPresets[presetIndex];
			settings.ambientColor = preset.color;
			settings.ambientIntensity = preset.intensity;
			GM_LOG("[Atmosphere] Sunlight Preset=%ls, Color=(%.2f, %.2f, %.2f), Intensity=%.2f", preset.name, preset.color.x, preset.color.y, preset.color.z, preset.intensity);
		}

		struct SpotLightIntensityStats
		{
			uint32 count = 0;
			float minimum = 0.f;
			float maximum = 0.f;
		};

		SpotLightIntensityStats GetSpotLightIntensityStats(const Scene& scene)
		{
			SpotLightIntensityStats stats{};
			stats.minimum = std::numeric_limits<float>::max();
			scene.ForEachGameObject([&stats](const GameObject& gameObject)
			{
				gameObject.ForEachComponent([&stats](const Component& component)
				{
					const SpotLightComponent* spotLight = dynamic_cast<const SpotLightComponent*>(&component);
					if (spotLight == nullptr)
						return;
					const float intensity = spotLight->GetIntensity();
					stats.minimum = std::min(stats.minimum, intensity);
					stats.maximum = std::max(stats.maximum, intensity);
					++stats.count;
				});
			});
			if (stats.count == 0)
				stats.minimum = 0.f;
			return stats;
		}

		void AdjustSpotLightIntensities(Scene& scene, float amount)
		{
			scene.ForEachGameObject([amount](GameObject& gameObject)
			{
				gameObject.ForEachComponent([amount](Component& component)
				{
					SpotLightComponent* spotLight = dynamic_cast<SpotLightComponent*>(&component);
					if (spotLight)
						spotLight->SetIntensity(std::clamp(spotLight->GetIntensity() + amount, 0.f, 20.f));
				});
			});
		}

		void AdjustAtmosphereSetting(Scene& scene, AtmosphereSetting setting, float direction, bool coarseAdjustment)
		{
			SceneAmbientSettings ambientSettings = scene.GetAmbientSettings();
			DepthFogSettings fogSettings = scene.GetDepthFogSettings();
			const float stepScale = coarseAdjustment ? 5.f : 1.f;

			switch (setting)
			{
			case AtmosphereSetting::SunlightPreset: SelectSunlightPreset(ambientSettings, direction); break;
			case AtmosphereSetting::AmbientIntensity: ambientSettings.ambientIntensity = std::clamp(ambientSettings.ambientIntensity + direction * 0.02f * stepScale, 0.f, 10.f); break;
			case AtmosphereSetting::AmbientColorR: ambientSettings.ambientColor.x = std::clamp(ambientSettings.ambientColor.x + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case AtmosphereSetting::AmbientColorG: ambientSettings.ambientColor.y = std::clamp(ambientSettings.ambientColor.y + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case AtmosphereSetting::AmbientColorB: ambientSettings.ambientColor.z = std::clamp(ambientSettings.ambientColor.z + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case AtmosphereSetting::SpotLightIntensity: AdjustSpotLightIntensities(scene, direction * 0.1f * stepScale); break;
			case AtmosphereSetting::FogEnabled: fogSettings.enabled = direction > 0.f; break;
			case AtmosphereSetting::FogColorR: fogSettings.fogColor.x = std::clamp(fogSettings.fogColor.x + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case AtmosphereSetting::FogColorG: fogSettings.fogColor.y = std::clamp(fogSettings.fogColor.y + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case AtmosphereSetting::FogColorB: fogSettings.fogColor.z = std::clamp(fogSettings.fogColor.z + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case AtmosphereSetting::FogStartDistance: fogSettings.startDistance = std::clamp(fogSettings.startDistance + direction * stepScale, 0.f, std::max(0.f, fogSettings.endDistance - 0.1f)); break;
			case AtmosphereSetting::FogEndDistance: fogSettings.endDistance = std::clamp(fogSettings.endDistance + direction * stepScale, fogSettings.startDistance + 0.1f, 5000.f); break;
			case AtmosphereSetting::FogDensity: fogSettings.density = std::clamp(fogSettings.density + direction * 0.1f * stepScale, 0.f, 20.f); break;
			default: break;
			}

			scene.SetAmbientSettings(ambientSettings);
			scene.SetDepthFogSettings(fogSettings);
		}

		void AdjustPostProcessSetting(Scene& scene, Renderer& renderer, PostProcessSetting setting, float direction, bool coarseAdjustment)
		{
			ToneMappingSettings toneMappingSettings = scene.GetToneMappingSettings();
			BloomSettings bloomSettings = renderer.GetBloomSettings();
			SSAOSettings ssaoSettings = renderer.GetSSAOSettings();
			ScreenSpaceOutlineSettings outlineSettings = renderer.GetScreenSpaceOutlineSettings();
			const float stepScale = coarseAdjustment ? 5.f : 1.f;

			switch (setting)
			{
			case PostProcessSetting::Exposure: toneMappingSettings.exposure = std::clamp(toneMappingSettings.exposure + direction * 0.1f * stepScale, -10.f, 10.f); break;
			case PostProcessSetting::BloomThreshold: bloomSettings.threshold = std::clamp(bloomSettings.threshold + direction * 0.1f * stepScale, 0.f, 20.f); break;
			case PostProcessSetting::BloomSoftKnee: bloomSettings.softKnee = std::clamp(bloomSettings.softKnee + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case PostProcessSetting::BloomIntensity: bloomSettings.intensity = std::clamp(bloomSettings.intensity + direction * 0.1f * stepScale, 0.f, 10.f); break;
			case PostProcessSetting::SSAORadius: ssaoSettings.radius = std::clamp(ssaoSettings.radius + direction * 0.01f * stepScale, 0.01f, 5.f); break;
			case PostProcessSetting::SSAOFadeStart: ssaoSettings.fadeStart = std::clamp(ssaoSettings.fadeStart + direction * 0.01f * stepScale, 0.f, std::max(0.f, ssaoSettings.fadeEnd - 0.001f)); break;
			case PostProcessSetting::SSAOFadeEnd: ssaoSettings.fadeEnd = std::clamp(ssaoSettings.fadeEnd + direction * 0.01f * stepScale, ssaoSettings.fadeStart + 0.001f, 10.f); break;
			case PostProcessSetting::SSAOSurfaceEpsilon: ssaoSettings.surfaceEpsilon = std::clamp(ssaoSettings.surfaceEpsilon + direction * 0.001f * stepScale, 0.f, 1.f); break;
			case PostProcessSetting::SSAOStrength: ssaoSettings.strength = std::clamp(ssaoSettings.strength + direction * 0.05f * stepScale, 0.f, 10.f); break;
			case PostProcessSetting::OutlineEnabled: renderer.SetScreenSpaceOutlineEnabled(direction > 0.f); break;
			case PostProcessSetting::OutlineDepthThreshold: outlineSettings.depthThreshold = std::clamp(outlineSettings.depthThreshold + direction * 0.0005f * stepScale, 0.0001f, 1.f); break;
			case PostProcessSetting::OutlineNormalThreshold: outlineSettings.normalThreshold = std::clamp(outlineSettings.normalThreshold + direction * 0.01f * stepScale, 0.f, 1.f); break;
			case PostProcessSetting::OutlineThickness: outlineSettings.thickness = std::clamp(outlineSettings.thickness + direction * 0.05f * stepScale, 0.1f, 5.f); break;
			case PostProcessSetting::OutlineColorR: outlineSettings.color.x = std::clamp(outlineSettings.color.x + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case PostProcessSetting::OutlineColorG: outlineSettings.color.y = std::clamp(outlineSettings.color.y + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case PostProcessSetting::OutlineColorB: outlineSettings.color.z = std::clamp(outlineSettings.color.z + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case PostProcessSetting::OutlineOpacity: outlineSettings.color.w = std::clamp(outlineSettings.color.w + direction * 0.02f * stepScale, 0.f, 1.f); break;
			case PostProcessSetting::FXAAEnabled: renderer.SetFXAAEnabled(direction > 0.f); break;
			default: break;
			}

			scene.SetToneMappingSettings(toneMappingSettings);
			renderer.SetBloomSettings(bloomSettings);
			renderer.SetSSAOSettings(ssaoSettings);
			renderer.SetScreenSpaceOutlineSettings(outlineSettings);
		}

		size_t FindShadowResolutionPreset(uint32 resolution)
		{
			size_t closestIndex = 0;
			uint32 closestDistance = std::numeric_limits<uint32>::max();
			for (size_t index = 0; index < ShadowResolutionPresets.size(); ++index)
			{
				const uint32 preset = ShadowResolutionPresets[index];
				const uint32 distance = preset > resolution ? preset - resolution : resolution - preset;
				if (distance < closestDistance)
				{
					closestIndex = index;
					closestDistance = distance;
				}
			}
			return closestIndex;
		}

		void AdjustShadowSetting(Renderer& renderer, ShadowSetting setting, float direction, bool coarseAdjustment)
		{
			const ShadowSettings& settings = renderer.GetShadowSettings();
			const float stepScale = coarseAdjustment ? 5.f : 1.f;
			const int32 integerDirection = direction > 0.f ? 1 : -1;
			switch (setting)
			{
			case ShadowSetting::CascadeColors: renderer.SetCascadeDebugColorsEnabled(direction > 0.f); break;
			case ShadowSetting::Resolution:
			{
				const int32 currentIndex = static_cast<int32>(FindShadowResolutionPreset(settings.mapResolution));
				const int32 nextIndex = std::clamp(currentIndex + integerDirection, 0, static_cast<int32>(ShadowResolutionPresets.size()) - 1);
				renderer.SetShadowResolution(ShadowResolutionPresets[nextIndex]);
				break;
			}
			case ShadowSetting::CascadeCount: renderer.SetShadowCascadeCount(static_cast<uint32>(std::clamp(static_cast<int32>(settings.cascadeCount) + integerDirection, 1, static_cast<int32>(MaxShadowCascadeCount)))); break;
			case ShadowSetting::PCFRadius: renderer.SetShadowPCFRadius(static_cast<uint32>(std::clamp(static_cast<int32>(settings.pcfRadius) + integerDirection, 0, 2))); break;
			case ShadowSetting::Strength: renderer.SetShadowStrength(settings.shadowStrength + direction * 0.02f * stepScale); break;
			case ShadowSetting::SplitLambda: renderer.SetShadowCascadeSplitLambda(settings.cascadeSplitLambda + direction * 0.02f * stepScale); break;
			case ShadowSetting::ColorR:
			case ShadowSetting::ColorG:
			case ShadowSetting::ColorB:
			{
				Color color = settings.shadowColor;
				float* channel = setting == ShadowSetting::ColorR ? &color.x : setting == ShadowSetting::ColorG ? &color.y : &color.z;
				*channel = std::clamp(*channel + direction * 0.02f * stepScale, 0.f, 1.f);
				renderer.SetShadowColor(color);
				break;
			}
			case ShadowSetting::DepthBias: renderer.SetShadowDepthBias(settings.depthBias + direction * 0.0001f * stepScale); break;
			case ShadowSetting::NormalBias: renderer.SetShadowNormalBias(settings.normalBias + direction * 0.001f * stepScale); break;
			case ShadowSetting::MaxDistance: renderer.SetShadowMaxDistance(settings.maxDistance + direction * stepScale); break;
			default: break;
			}
		}

		void PrintAtmosphereSettings(const Scene& scene)
		{
			const SceneAmbientSettings& ambient = scene.GetAmbientSettings();
			const DepthFogSettings& fog = scene.GetDepthFogSettings();
			const SpotLightIntensityStats spotLights = GetSpotLightIntensityStats(scene);
			GM_LOG("[Atmosphere] Sunlight Preset=%ls", GetSunlightPresetName(ambient));
			GM_LOG("[Atmosphere] Ambient Color=(%.2f, %.2f, %.2f), Intensity=%.2f", ambient.ambientColor.x, ambient.ambientColor.y, ambient.ambientColor.z, ambient.ambientIntensity);
			GM_LOG("[Atmosphere] Spot Lights Count=%u, Intensity Min=%.2f, Max=%.2f", spotLights.count, spotLights.minimum, spotLights.maximum);
			GM_LOG("[Atmosphere] Fog Enabled=%d, Color=(%.2f, %.2f, %.2f), Start=%.2f, End=%.2f, Density=%.2f", fog.enabled, fog.fogColor.x, fog.fogColor.y, fog.fogColor.z, fog.startDistance, fog.endDistance, fog.density);
		}

		void PrintPostProcessSettings(const Scene& scene, const Renderer& renderer)
		{
			const ToneMappingSettings& toneMapping = scene.GetToneMappingSettings();
			const BloomSettings& bloom = renderer.GetBloomSettings();
			const SSAOSettings& ssao = renderer.GetSSAOSettings();
			const ScreenSpaceOutlineSettings& outline = renderer.GetScreenSpaceOutlineSettings();
			GM_LOG("[Post Process] Exposure=%.2f", toneMapping.exposure);
			GM_LOG("[Post Process] Bloom Threshold=%.2f, SoftKnee=%.2f, Intensity=%.2f", bloom.threshold, bloom.softKnee, bloom.intensity);
			GM_LOG("[Post Process] SSAO Radius=%.3f, FadeStart=%.3f, FadeEnd=%.3f, SurfaceEpsilon=%.4f, Strength=%.2f", ssao.radius, ssao.fadeStart, ssao.fadeEnd, ssao.surfaceEpsilon, ssao.strength);
			GM_LOG("[Post Process] Outline Enabled=%d, DepthThreshold=%.4f, NormalThreshold=%.2f, Thickness=%.2f, Color=(%.2f, %.2f, %.2f, %.2f)", renderer.IsScreenSpaceOutlineEnabled(), outline.depthThreshold, outline.normalThreshold, outline.thickness, outline.color.x, outline.color.y, outline.color.z, outline.color.w);
			GM_LOG("[Post Process] FXAA Enabled=%d", renderer.IsFXAAEnabled());
		}

		void PrintShadowSettings(const Renderer& renderer)
		{
			const ShadowSettings& settings = renderer.GetShadowSettings();
			GM_LOG("[Shadow] Cascade Colors=%d, Resolution=%u, Cascade Count=%u, PCF Radius=%u", renderer.IsCascadeDebugColorsEnabled(), settings.mapResolution, settings.cascadeCount, settings.pcfRadius);
			GM_LOG("[Shadow] Strength=%.2f, Split Lambda=%.2f, Distance=%.1f", settings.shadowStrength, settings.cascadeSplitLambda, settings.maxDistance);
			GM_LOG("[Shadow] Color=(%.2f, %.2f, %.2f), Depth Bias=%.4f, Normal Bias=%.4f", settings.shadowColor.x, settings.shadowColor.y, settings.shadowColor.z, settings.depthBias, settings.normalBias);
		}

		std::wstring CreateBeatProgressBar(float progress)
		{
			const uint32 filledCount = static_cast<uint32>(progress * BeatProgressBarWidth);
			std::wstring progressBar(BeatProgressBarWidth, L'-');
			std::fill_n(progressBar.begin(), std::min(filledCount, BeatProgressBarWidth), L'#');
			return progressBar;
		}

		const wchar_t* GetRenderTargetDebugViewName(RenderTargetDebugView view)
		{
			switch (view)
			{
			case RenderTargetDebugView::OriginalScene: return L"Original Scene";
			case RenderTargetDebugView::BaseColor: return L"G-Buffer BaseColor";
			case RenderTargetDebugView::WorldNormal: return L"G-Buffer WorldNormal";
			case RenderTargetDebugView::AmbientOcclusion: return L"G-Buffer AmbientOcclusion";
			case RenderTargetDebugView::ScreenSpaceAmbientOcclusion: return L"Screen Space AmbientOcclusion";
			case RenderTargetDebugView::ScreenSpaceOutline: return L"Screen Space Outline";
			case RenderTargetDebugView::MaterialFlags: return L"G-Buffer MaterialFlags";
			case RenderTargetDebugView::Emissive: return L"G-Buffer Emissive";
			case RenderTargetDebugView::SceneDepth: return L"SceneDepth";
			case RenderTargetDebugView::BloomContribution: return L"Bloom Contribution";
			case RenderTargetDebugView::SceneColorA: return L"HDR SceneColor A";
			case RenderTargetDebugView::SceneColorB: return L"HDR SceneColor B";
			default: return L"Unknown";
			}
		}

	}

	DebugTextWidget::~DebugTextWidget()
	{
		RestoreMaterialHighlight();
	}

	void DebugTextWidget::RestoreMaterialHighlight()
	{
		if (_hasMaterialHighlight == false)
			return;
		GameObject* gameObject = _selectedMaterialObject.Get();
		Material* material = gameObject ? GetMaterial(*gameObject, _materialSlotIndex) : nullptr;
		if (material)
			material->SetColorData(_materialOriginalColorData);
		_materialHighlightTime = 0.f;
		_hasMaterialHighlight = false;
	}

	void DebugTextWidget::ApplyMaterialHighlight()
	{
		GameObject* gameObject = _selectedMaterialObject.Get();
		Material* material = gameObject ? GetMaterial(*gameObject, _materialSlotIndex) : nullptr;
		if (material == nullptr)
			return;
		_materialOriginalColorData = material->GetColorData();
		material->SetColorBlend(Colors::Magenta, 0.35f);
		_materialHighlightTime = MaterialHighlightDuration;
		_hasMaterialHighlight = true;
	}

	void DebugTextWidget::InitializeAudioDebug()
	{
		const std::span<const RhythmBGMDesc* const> bgms = GetRhythmBGMs();
		if (_audioSongOffsets.size() != bgms.size())
		{
			_audioSongOffsets.clear();
			_audioSongOffsets.reserve(bgms.size());
			for (const RhythmBGMDesc* bgm : bgms)
				_audioSongOffsets.push_back(bgm->songOffsetSeconds);
		}

		const RhythmBGMDesc* currentBGM = GetCurrentRhythmBGM();
		for (uint32 index = 0; index < bgms.size(); ++index)
		{
			if (bgms[index] != currentBGM)
				continue;
			_audioBGMIndex = index;
			_audioSongOffsets[index] = GetCurrentRhythmBGMSongOffset();
			break;
		}
		ResetAudioInputSamples();
	}

	void DebugTextWidget::ChangeAudioBGM(int32 direction)
	{
		const std::span<const RhythmBGMDesc* const> bgms = GetRhythmBGMs();
		if (bgms.empty())
			return;
		const int32 bgmCount = static_cast<int32>(bgms.size());
		_audioBGMIndex = static_cast<uint32>((static_cast<int32>(_audioBGMIndex) + direction + bgmCount) % bgmCount);
		PlaySelectedAudioBGM();
	}

	void DebugTextWidget::PlaySelectedAudioBGM()
	{
		const std::span<const RhythmBGMDesc* const> bgms = GetRhythmBGMs();
		if (_audioBGMIndex >= bgms.size() || _audioBGMIndex >= _audioSongOffsets.size())
			return;
		const RhythmBGMDesc& bgm = *bgms[_audioBGMIndex];
		PlayRhythmBGM(bgm, _audioSongOffsets[_audioBGMIndex]);
		ResetAudioInputSamples();
		GM_LOG("[BGM Offset] BGM 변경: %ls, BPM: %.1f, Song Offset: %+.3fs", bgm.commonResourceKey, bgm.bpm, _audioSongOffsets[_audioBGMIndex]);
	}

	void DebugTextWidget::AdjustAudioSongOffset(float amount)
	{
		const std::span<const RhythmBGMDesc* const> bgms = GetRhythmBGMs();
		if (_audioBGMIndex >= bgms.size() || _audioBGMIndex >= _audioSongOffsets.size())
			return;
		_audioSongOffsets[_audioBGMIndex] = std::clamp(_audioSongOffsets[_audioBGMIndex] + amount, -5.f, 5.f);
		if (GetCurrentRhythmBGM() != bgms[_audioBGMIndex])
			PlaySelectedAudioBGM();
		else
		{
			SetCurrentRhythmBGMSongOffset(_audioSongOffsets[_audioBGMIndex]);
			ResetAudioInputSamples();
		}
		GM_LOG("[BGM Offset] %ls Song Offset: %+.3fs", bgms[_audioBGMIndex]->commonResourceKey, _audioSongOffsets[_audioBGMIndex]);
	}

	void DebugTextWidget::RecordAudioInput(bool strongAttack)
	{
		if (_beatSystem.HasPlaybackTime() == false)
		{
			GM_LOG("[BGM Offset] BGM이 재생 중이 아닙니다. Enter로 선택한 BGM을 재생해주세요.");
			return;
		}

		const RhythmJudgeResult result = HiFiRushStatics::GetRhythmJudge().Judge(_beatSystem, strongAttack ? RhythmInputType::StrongAttack : RhythmInputType::WeakAttack);
		++_audioInputSampleCount;
		_audioInputErrorSum += result.secError;
		const float averageError = _audioInputErrorSum / static_cast<float>(_audioInputSampleCount);
		GM_LOG("[BGM Offset] %s Beat: %.3f, Error: %+.1fms, Judgment: %ls, Average: %+.1fms (%u회), 추천: %ls", strongAttack ? "Right" : "Left", result.inputBeat, result.secError * 1000.f, GetRhythmJudgeGradeName(result.judgeGrade), averageError * 1000.f, _audioInputSampleCount, GetAudioOffsetRecommendation(averageError));
	}

	void DebugTextWidget::ResetAudioInputSamples()
	{
		_audioInputSampleCount = 0;
		_audioInputErrorSum = 0.f;
	}

	std::unique_ptr<Widget> DebugTextWidget::BuildWidgetTree()
	{
		auto textBlock = CreateRootWidget<TextBlock>();
		textBlock->SetPosition(Vector2{ 0.f, 0.f });
		textBlock->SetColor(Colors::Green);

		return textBlock;
	}

	void DebugTextWidget::OnTick(float deltaTime)
	{
		const Input& input = APPLICATION.GetInput();
		const Page previousPage = _page;
		const bool isControlPressed = input.IsKeyRepeat(KeyCode::LeftCtrl) || input.IsKeyRepeat(KeyCode::RightCtrl);
		const bool isMaterialPageRequested = isControlPressed && input.IsKeyDown(KeyCode::F6);
		if (isControlPressed && input.IsKeyDown(KeyCode::F1))
			_page = Page::General;
		else if (isControlPressed && input.IsKeyDown(KeyCode::F2))
			_page = Page::Rendering;
		else if (isControlPressed && input.IsKeyDown(KeyCode::F3))
			_page = Page::Shadow;
		else if (isControlPressed && input.IsKeyDown(KeyCode::F4))
			_page = Page::Atmosphere;
		else if (isControlPressed && input.IsKeyDown(KeyCode::F5))
			_page = Page::PostProcess;
		else if (isControlPressed && input.IsKeyDown(KeyCode::F6))
			_page = Page::Material;
		else if (isControlPressed && input.IsKeyDown(KeyCode::F7))
			_page = Page::Audio;
		if (previousPage != _page)
		{
			if (previousPage == Page::Material)
				RestoreMaterialHighlight();
			if (_page == Page::Material)
				ApplyMaterialHighlight();
			if (_page == Page::Audio)
				InitializeAudioDebug();
		}
		if (_selectedMaterialObject.IsValid() == false)
		{
			_selectedEnvironmentModelIndex.reset();
			_selectedMaterialObject.Reset();
			_hasMaterialHighlight = false;
		}
		if (_hasMaterialHighlight)
		{
			_materialHighlightTime = std::max(0.f, _materialHighlightTime - deltaTime);
			if (_materialHighlightTime <= 0.f)
				RestoreMaterialHighlight();
		}

		Renderer& renderer = APPLICATION.GetRenderer();
		Scene* activeScene = APPLICATION.GetSceneManager().GetActiveScene();
		if (isMaterialPageRequested && activeScene)
		{
			GameObject* qamil = FindQamil(*activeScene);
			if (qamil)
			{
				RestoreMaterialHighlight();
				_selectedEnvironmentModelIndex.reset();
				_selectedMaterialObject = qamil->GetWeakPtr();
				_materialSlotIndex = 0;
				ApplyMaterialHighlight();
				GM_LOG("[Material Debug] Qamil 선택, Material Slots: %u", GetMaterialSlotCount(*qamil));
			}
		}
		if (APPLICATION.GetUIManager().AreDebugWidgetsVisible())
		{
			const bool isShiftPressed = input.IsKeyRepeat(KeyCode::LeftShift) || input.IsKeyRepeat(KeyCode::RightShift);
			if (input.IsKeyDown(KeyCode::Home))
				FillPlayerReverb(activeScene);
			if (_page == Page::General)
			{
				if (input.IsKeyDown(KeyCode::Up))
					_effectSettingIndex = (_effectSettingIndex + EffectSettingCount - 1) % EffectSettingCount;
				else if (input.IsKeyDown(KeyCode::Down))
					_effectSettingIndex = (_effectSettingIndex + 1) % EffectSettingCount;
				const EffectSetting selectedSetting = static_cast<EffectSetting>(_effectSettingIndex);
				if (input.IsKeyDown(KeyCode::Left))
					AdjustEffectSetting(activeScene, selectedSetting, -1.f, isShiftPressed);
				else if (input.IsKeyDown(KeyCode::Right))
					AdjustEffectSetting(activeScene, selectedSetting, 1.f, isShiftPressed);
			}
			if (_page == Page::Shadow)
			{
				if (input.IsKeyDown(KeyCode::Up))
					_shadowSettingIndex = (_shadowSettingIndex + ShadowSettingCount - 1) % ShadowSettingCount;
				else if (input.IsKeyDown(KeyCode::Down))
					_shadowSettingIndex = (_shadowSettingIndex + 1) % ShadowSettingCount;
				const ShadowSetting selectedSetting = static_cast<ShadowSetting>(_shadowSettingIndex);
				if (input.IsKeyDown(KeyCode::Left))
					AdjustShadowSetting(renderer, selectedSetting, -1.f, isShiftPressed);
				else if (input.IsKeyDown(KeyCode::Right))
					AdjustShadowSetting(renderer, selectedSetting, 1.f, isShiftPressed);
				else if (input.IsKeyDown(KeyCode::Enter) && selectedSetting == ShadowSetting::CascadeColors)
					renderer.SetCascadeDebugColorsEnabled(renderer.IsCascadeDebugColorsEnabled() == false);
				if (input.IsKeyDown(KeyCode::P))
					PrintShadowSettings(renderer);
			}
			else if (_page == Page::Atmosphere && activeScene)
			{
				if (input.IsKeyDown(KeyCode::Up))
					_atmosphereSettingIndex = (_atmosphereSettingIndex + AtmosphereSettingCount - 1) % AtmosphereSettingCount;
				else if (input.IsKeyDown(KeyCode::Down))
					_atmosphereSettingIndex = (_atmosphereSettingIndex + 1) % AtmosphereSettingCount;
				const AtmosphereSetting selectedSetting = static_cast<AtmosphereSetting>(_atmosphereSettingIndex);
				if (input.IsKeyDown(KeyCode::Left))
					AdjustAtmosphereSetting(*activeScene, selectedSetting, -1.f, isShiftPressed);
				else if (input.IsKeyDown(KeyCode::Right))
					AdjustAtmosphereSetting(*activeScene, selectedSetting, 1.f, isShiftPressed);
				else if (input.IsKeyDown(KeyCode::Enter) && selectedSetting == AtmosphereSetting::FogEnabled)
					AdjustAtmosphereSetting(*activeScene, selectedSetting, activeScene->GetDepthFogSettings().enabled ? -1.f : 1.f, false);
				if (input.IsKeyDown(KeyCode::P))
					PrintAtmosphereSettings(*activeScene);
			}
			else if (_page == Page::PostProcess && activeScene)
			{
				if (input.IsKeyDown(KeyCode::Up))
					_postProcessSettingIndex = (_postProcessSettingIndex + PostProcessSettingCount - 1) % PostProcessSettingCount;
				else if (input.IsKeyDown(KeyCode::Down))
					_postProcessSettingIndex = (_postProcessSettingIndex + 1) % PostProcessSettingCount;
				const PostProcessSetting selectedSetting = static_cast<PostProcessSetting>(_postProcessSettingIndex);
				if (input.IsKeyDown(KeyCode::Left))
					AdjustPostProcessSetting(*activeScene, renderer, selectedSetting, -1.f, isShiftPressed);
				else if (input.IsKeyDown(KeyCode::Right))
					AdjustPostProcessSetting(*activeScene, renderer, selectedSetting, 1.f, isShiftPressed);
				else if (input.IsKeyDown(KeyCode::Enter) && selectedSetting == PostProcessSetting::OutlineEnabled)
					renderer.SetScreenSpaceOutlineEnabled(renderer.IsScreenSpaceOutlineEnabled() == false);
				else if (input.IsKeyDown(KeyCode::Enter) && selectedSetting == PostProcessSetting::FXAAEnabled)
					renderer.SetFXAAEnabled(renderer.IsFXAAEnabled() == false);
				if (input.IsKeyDown(KeyCode::P))
					PrintPostProcessSettings(*activeScene, renderer);
			}
			else if (_page == Page::Material && activeScene)
			{
				if (input.IsMouseDown(MouseButton::Middle))
				{
					GameObject* selectedObject = nullptr;
					std::optional<uint32> modelIndex{};
					uint32 materialSlotCount = 0;
					if (SelectMeshByRay(*activeScene, input.GetMousePosition(), _selectedMaterialObject.Get(), selectedObject, modelIndex, materialSlotCount))
					{
						RestoreMaterialHighlight();
						_selectedEnvironmentModelIndex = modelIndex;
						_selectedMaterialObject = selectedObject->GetWeakPtr();
						_materialSlotIndex = 0;
						ApplyMaterialHighlight();
					}
				}
				GameObject* selectedObject = _selectedMaterialObject.Get();
				if (selectedObject)
				{
					const uint32 materialSlotCount = GetMaterialSlotCount(*selectedObject);
					if (materialSlotCount > 0)
					{
						if (input.IsKeyDown(KeyCode::Up))
						{
							RestoreMaterialHighlight();
							_materialSlotIndex = (_materialSlotIndex + materialSlotCount - 1) % materialSlotCount;
							ApplyMaterialHighlight();
						}
						else if (input.IsKeyDown(KeyCode::Down))
						{
							RestoreMaterialHighlight();
							_materialSlotIndex = (_materialSlotIndex + 1) % materialSlotCount;
							ApplyMaterialHighlight();
						}
						const float adjustment = isShiftPressed ? 0.5f : 0.05f;
						if (input.IsKeyDown(KeyCode::Left))
							AdjustMaterialEmissiveIntensity(*activeScene, *selectedObject, _selectedEnvironmentModelIndex, _materialSlotIndex, -adjustment);
						else if (input.IsKeyDown(KeyCode::Right))
							AdjustMaterialEmissiveIntensity(*activeScene, *selectedObject, _selectedEnvironmentModelIndex, _materialSlotIndex, adjustment);
					}
				}
			}
			else if (_page == Page::Audio)
			{
				if (input.IsKeyDown(KeyCode::Up))
					ChangeAudioBGM(-1);
				else if (input.IsKeyDown(KeyCode::Down))
					ChangeAudioBGM(1);
				if (input.IsKeyDown(KeyCode::Enter))
					PlaySelectedAudioBGM();
				const float offsetStep = isShiftPressed ? AudioOffsetCoarseStep : AudioOffsetStep;
				if (input.IsKeyDown(KeyCode::LBracket))
					AdjustAudioSongOffset(-offsetStep);
				else if (input.IsKeyDown(KeyCode::RBracket))
					AdjustAudioSongOffset(offsetStep);
				if (input.IsMouseDown(MouseButton::Left))
					RecordAudioInput(false);
				if (input.IsMouseDown(MouseButton::Right))
					RecordAudioInput(true);
				if (input.IsKeyDown(KeyCode::P) && _audioBGMIndex < GetRhythmBGMs().size() && _audioBGMIndex < _audioSongOffsets.size())
				{
					const float averageError = _audioInputSampleCount > 0 ? _audioInputErrorSum / static_cast<float>(_audioInputSampleCount) : 0.f;
					GM_LOG("[BGM Offset] 최종값: %ls = %+.3fs, Average Error: %+.1fms (%u회)", GetRhythmBGMs()[_audioBGMIndex]->commonResourceKey, _audioSongOffsets[_audioBGMIndex], averageError * 1000.f, _audioInputSampleCount);
				}
			}
		}

		_accTime += deltaTime;
		++_callCount;

		if (_accTime >= 1.f)
		{
			_fps = static_cast<float>(_callCount) / _accTime;
			_callCount = 0;
			_accTime = 0.f;
		}

		if (_beatSystem.DidCrossBeatBoundary())
			_beatPulseTime = BeatPulseDuration;
		else
			_beatPulseTime = std::max(0.f, _beatPulseTime - deltaTime);

		TextBlock* textBlock = static_cast<TextBlock*>(GetRootWidget());
		textBlock->SetColor(_beatPulseTime > 0.f ? Colors::Red : Colors::Green);

		std::wostringstream text;
		text << std::fixed << std::setprecision(1);
		const wchar_t* pageName = L"General";
		if (_page == Page::Rendering)
			pageName = L"Rendering";
		else if (_page == Page::Shadow)
			pageName = L"Shadow";
		else if (_page == Page::Atmosphere)
			pageName = L"Atmosphere";
		else if (_page == Page::PostProcess)
			pageName = L"Post Process";
		else if (_page == Page::Material)
			pageName = L"Material";
		else if (_page == Page::Audio)
			pageName = L"Audio / BGM Offset";
		text << L"Debug Page [Ctrl+F1 ... Ctrl+F7] : " << pageName;
		text << L"\nFPS : " << _fps;
		if (_page == Page::General)
		{
			text << L"\nBPM : " << _beatSystem.GetBPM();
			if (_beatSystem.HasPlaybackTime() == false)
			{
				text << L"\nBGM Time : Not Playing";
				text << L"\nBeat : -";
				text << L"\nBeat Progress : [--------------------] 0.0%";
			}
			else
			{
				text << std::setprecision(3);
				text << L"\nBGM Time : " << _beatSystem.GetPlaybackTime() << L" s";
				text << L"\nBeat : " << _beatSystem.GetCurrentBeatIndex() << L" (" << _beatSystem.GetCurrentBeat() << L')';
				text << std::setprecision(1);
				text << L"\nBeat Progress : [" << CreateBeatProgressBar(_beatSystem.GetBeatProgress()) << L"] " << _beatSystem.GetBeatProgress() * 100.f << L'%';
			}

			const PhysicsSystem& physicsSystem = APPLICATION.GetPhysicsSystem();
			const ReverbComponent* reverbComponent = FindPlayerReverbComponent(activeScene);
			const std::optional<float> weakAttackEmissiveIntensity = HiFiRushStatics::GetEffectPresets().FindEmissiveIntensity(WeakAttackRotationCircleResourceKey);
			const std::optional<float> weakAttackBeatHitEmissiveIntensity = HiFiRushStatics::GetEffectPresets().FindEmissiveIntensity(WeakAttackBeatHitEffectId, WeakAttackBeatHitResourceKey);
			const std::optional<float> weakAttackBeatHitFloorEmissiveIntensity = HiFiRushStatics::GetEffectPresets().FindEmissiveIntensity(WeakAttackBeatHitEffectId, WeakAttackBeatHitFloorResourceKey);
			const std::optional<float> strongAttackEmissiveIntensity = FindStrongAttackMainEmissiveIntensity();
			const EffectPresets& effectPresets = HiFiRushStatics::GetEffectPresets();
			const std::optional<float> swordChargeEmissiveIntensity = effectPresets.FindEmissiveIntensity(SwordChargeResourceKey);
			const std::optional<float> swordSlashEmissiveIntensity = effectPresets.FindEmissiveIntensity(SwordSlashResourceKey);
			const std::optional<float> stumpFloorEmissiveIntensity = effectPresets.FindEmissiveIntensity(StumpEffectId, StumpFloorResourceKey);
			const std::optional<float> stumpCloudEmissiveIntensity = effectPresets.FindEmissiveIntensity(StumpEffectId, StumpCloudResourceKey);
			const std::optional<float> stumpSphereEmissiveIntensity = effectPresets.FindEmissiveIntensity(StumpEffectId, StumpSphereResourceKey);
			const std::optional<float> qamilStumpFloorEmissiveIntensity = effectPresets.FindEmissiveIntensity(QamilStumpFloorEffectId, QamilStumpFloorResourceKey);
			const std::optional<float> hibikiCloudEmissiveIntensity = effectPresets.FindEmissiveIntensity(HibikiCloudEffectId, HibikiCloudResourceKey);
			const ChiEffectComponent* chiEffectComponent = FindChiEffectComponent(activeScene);
			const float afterImageEmissiveIntensity = chiEffectComponent ? chiEffectComponent->GetAfterImageEmissiveIntensity() : 0.f;
			std::array<Color, 3> afterImageColors{};
			if (chiEffectComponent)
			{
				for (uint32 index = 0; index < afterImageColors.size(); ++index)
					afterImageColors[index] = chiEffectComponent->GetAfterImageColor(index);
			}
			text << L"\n[Effect] Select [Up / Down], Adjust [Left / Right], Coarse [Shift]";
			auto appendEffectValue = [this, &text](EffectSetting setting, const wchar_t* name, float value)
			{
				text << L'\n' << (_effectSettingIndex == static_cast<uint32>(setting) ? L"> " : L"  ") << name << L" : " << std::setprecision(2) << value;
			};
			appendEffectValue(EffectSetting::WeakAttackEmissive, L"Weak Attack Circle Emissive", weakAttackEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::WeakAttackBeatHitEmissive, L"Weak Attack Beat Hit Emissive", weakAttackBeatHitEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::WeakAttackBeatHitFloorEmissive, L"Weak Attack Beat Hit Floor Emissive", weakAttackBeatHitFloorEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::StrongAttackEmissive, L"Strong Attack 1~3 Main Emissive", strongAttackEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::SwordChargeEmissive, L"Sword Charge Emissive", swordChargeEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::SwordSlashEmissive, L"Sword Slash Emissive", swordSlashEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::StumpFloorEmissive, L"Stump Floor Emissive", stumpFloorEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::StumpCloudEmissive, L"Stump Cloud Emissive", stumpCloudEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::StumpSphereEmissive, L"Stump Sphere Emissive", stumpSphereEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::QamilStumpFloorEmissive, L"Qamil Stump Floor Emissive", qamilStumpFloorEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::HibikiCloudEmissive, L"Hibiki Cloud Emissive", hibikiCloudEmissiveIntensity.value_or(0.f));
			appendEffectValue(EffectSetting::AfterImageEmissive, L"Dash After Image Emissive", afterImageEmissiveIntensity);
			appendEffectValue(EffectSetting::AfterImageOpacity, L"Dash After Image Opacity", chiEffectComponent ? chiEffectComponent->GetAfterImageOpacity() : 0.f);
			appendEffectValue(EffectSetting::AfterImageColorBlendRatio, L"Dash After Image Color Blend Ratio", chiEffectComponent ? chiEffectComponent->GetAfterImageColorBlendRatio() : 0.f);
			text << L"\n[After Image 1]";
			appendEffectValue(EffectSetting::AfterImageFirstColorR, L"Color R", afterImageColors[0].x);
			appendEffectValue(EffectSetting::AfterImageFirstColorG, L"Color G", afterImageColors[0].y);
			appendEffectValue(EffectSetting::AfterImageFirstColorB, L"Color B", afterImageColors[0].z);
			text << L"\n[After Image 2]";
			appendEffectValue(EffectSetting::AfterImageSecondColorR, L"Color R", afterImageColors[1].x);
			appendEffectValue(EffectSetting::AfterImageSecondColorG, L"Color G", afterImageColors[1].y);
			appendEffectValue(EffectSetting::AfterImageSecondColorB, L"Color B", afterImageColors[1].z);
			text << L"\n[After Image 3]";
			appendEffectValue(EffectSetting::AfterImageThirdColorR, L"Color R", afterImageColors[2].x);
			appendEffectValue(EffectSetting::AfterImageThirdColorG, L"Color G", afterImageColors[2].y);
			appendEffectValue(EffectSetting::AfterImageThirdColorB, L"Color B", afterImageColors[2].z);
			text << L"\nBounding Volume [B] : " << (renderer.IsBoundingVolumeDebugDrawEnabled() ? L"On" : L"Off");
			text << L"\n3D Collider [C] : " << (physicsSystem.IsCollider3DDebugDrawEnabled() ? L"On" : L"Off");
			text << L"\nNavigation Mesh [N] : " << (physicsSystem.GetNavMeshSystem().IsDebugDrawEnabled() ? L"On" : L"Off");
			text << L"\nReverb Fill [Home] : " << std::setprecision(1) << (reverbComponent ? reverbComponent->GetReverb() : 0.f) << L" / " << (reverbComponent ? reverbComponent->GetMaxReverb() : 0.f);
		}
		else if (_page == Page::Rendering)
		{
			const CullingDebugStats cullingStats = renderer.GetCullingDebugStats();
			const StaticMeshInstancingDebugStats instancingStats = renderer.GetStaticMeshInstancingDebugStats();
			const uint32 submittedCount = cullingStats.staticMesh.submittedCount + cullingStats.skeletalMesh.submittedCount;
			const uint32 visibleCount = cullingStats.staticMesh.visibleCount + cullingStats.skeletalMesh.visibleCount;
			const uint32 culledCount = cullingStats.staticMesh.culledCount + cullingStats.skeletalMesh.culledCount;
			text << L"\nFrustum Culling [Ctrl+B] : " << (renderer.IsFrustumCullingEnabled() ? L"On" : L"Off");
			text << L"\nStatic Mesh Instancing [Ctrl+I] : " << (renderer.IsStaticMeshInstancingEnabled() ? L"On" : L"Off");
			text << L"\nRender Target [F9 / Ctrl+F9 Reset] : " << GetRenderTargetDebugViewName(renderer.GetRenderTargetDebugView());
			text << L"\nStatic Mesh Batches : " << instancingStats.renderBatchCount;
			text << L"\nStatic Mesh Draw Calls (Normal / Instanced) : " << instancingStats.normalDrawCallCount << L" / " << instancingStats.instancedDrawCallCount;
			text << L"\nInstanced Objects : " << instancingStats.instancedInstanceCount;
			text << L"\nCulling Stats (Submitted / Visible / Culled)";
			text << L"\n  Total : " << submittedCount << L" / " << visibleCount << L" / " << culledCount;
			text << L"\n  Static : " << cullingStats.staticMesh.submittedCount << L" / " << cullingStats.staticMesh.visibleCount << L" / " << cullingStats.staticMesh.culledCount;
			text << L"\n  Skeletal : " << cullingStats.skeletalMesh.submittedCount << L" / " << cullingStats.skeletalMesh.visibleCount << L" / " << cullingStats.skeletalMesh.culledCount;
		}
		else if (_page == Page::Shadow)
		{
			const ShadowSettings& settings = renderer.GetShadowSettings();
			const uint32 pcfKernelSize = settings.pcfRadius * 2 + 1;
			text << L"\nSelect [Up / Down], Adjust [Left / Right], Coarse [Shift], Toggle [Enter], Print [P]";
			auto appendShadowValue = [this, &text](ShadowSetting setting, const wchar_t* name, float value, int32 precision)
			{
				text << L'\n' << (_shadowSettingIndex == static_cast<uint32>(setting) ? L"> " : L"  ") << name << L" : " << std::setprecision(precision) << value;
			};
			text << L'\n' << (_shadowSettingIndex == static_cast<uint32>(ShadowSetting::CascadeColors) ? L"> " : L"  ") << L"Cascade Colors : " << (renderer.IsCascadeDebugColorsEnabled() ? L"On" : L"Off");
			text << L'\n' << (_shadowSettingIndex == static_cast<uint32>(ShadowSetting::Resolution) ? L"> " : L"  ") << L"Resolution : " << settings.mapResolution << L" x " << settings.mapResolution;
			text << L'\n' << (_shadowSettingIndex == static_cast<uint32>(ShadowSetting::CascadeCount) ? L"> " : L"  ") << L"Cascade Count : " << settings.cascadeCount;
			text << L'\n' << (_shadowSettingIndex == static_cast<uint32>(ShadowSetting::PCFRadius) ? L"> " : L"  ") << L"PCF : " << pcfKernelSize << L" x " << pcfKernelSize << L" (" << pcfKernelSize * pcfKernelSize << L" Samples)";
			appendShadowValue(ShadowSetting::Strength, L"Strength", settings.shadowStrength, 2);
			appendShadowValue(ShadowSetting::SplitLambda, L"Split Lambda (0=Uniform, 1=Logarithmic)", settings.cascadeSplitLambda, 2);
			appendShadowValue(ShadowSetting::ColorR, L"Color R", settings.shadowColor.x, 2);
			appendShadowValue(ShadowSetting::ColorG, L"Color G", settings.shadowColor.y, 2);
			appendShadowValue(ShadowSetting::ColorB, L"Color B", settings.shadowColor.z, 2);
			appendShadowValue(ShadowSetting::DepthBias, L"Depth Bias", settings.depthBias, 4);
			appendShadowValue(ShadowSetting::NormalBias, L"Normal Bias", settings.normalBias, 4);
			appendShadowValue(ShadowSetting::MaxDistance, L"Distance", settings.maxDistance, 1);
		}
		else if (_page == Page::Atmosphere && activeScene)
		{
			const SceneAmbientSettings& ambient = activeScene->GetAmbientSettings();
			const DepthFogSettings& fog = activeScene->GetDepthFogSettings();
			const SpotLightIntensityStats spotLights = GetSpotLightIntensityStats(*activeScene);
			text << L"\nSelect [Up / Down], Adjust [Left / Right], Coarse [Shift], Toggle [Enter], Print [P]";

			auto appendValue = [this, &text](AtmosphereSetting setting, const wchar_t* name, float value, int32 precision)
			{
				text << L'\n' << (_atmosphereSettingIndex == static_cast<uint32>(setting) ? L"> " : L"  ") << name << L" : " << std::setprecision(precision) << value;
			};
			auto appendState = [this, &text](AtmosphereSetting setting, const wchar_t* name, bool enabled)
			{
				text << L'\n' << (_atmosphereSettingIndex == static_cast<uint32>(setting) ? L"> " : L"  ") << name << L" : " << (enabled ? L"On" : L"Off");
			};

			text << L"\n[Ambient]";
			text << L'\n' << (_atmosphereSettingIndex == static_cast<uint32>(AtmosphereSetting::SunlightPreset) ? L"> " : L"  ") << L"Sunlight Preset : " << GetSunlightPresetName(ambient);
			appendValue(AtmosphereSetting::AmbientIntensity, L"Intensity", ambient.ambientIntensity, 2);
			appendValue(AtmosphereSetting::AmbientColorR, L"Color R", ambient.ambientColor.x, 2);
			appendValue(AtmosphereSetting::AmbientColorG, L"Color G", ambient.ambientColor.y, 2);
			appendValue(AtmosphereSetting::AmbientColorB, L"Color B", ambient.ambientColor.z, 2);
			text << L"\n[Spot Lights]";
			appendValue(AtmosphereSetting::SpotLightIntensity, spotLights.count > 1 ? L"Intensity Min" : L"Intensity", spotLights.minimum, 2);
			if (spotLights.maximum != spotLights.minimum)
				text << L" / Max : " << std::setprecision(2) << spotLights.maximum;
			text << L"\n[Fog]";
			appendState(AtmosphereSetting::FogEnabled, L"Enabled", fog.enabled);
			appendValue(AtmosphereSetting::FogColorR, L"Color R", fog.fogColor.x, 2);
			appendValue(AtmosphereSetting::FogColorG, L"Color G", fog.fogColor.y, 2);
			appendValue(AtmosphereSetting::FogColorB, L"Color B", fog.fogColor.z, 2);
			appendValue(AtmosphereSetting::FogStartDistance, L"Start Distance", fog.startDistance, 1);
			appendValue(AtmosphereSetting::FogEndDistance, L"End Distance", fog.endDistance, 1);
			appendValue(AtmosphereSetting::FogDensity, L"Density", fog.density, 2);
		}
		else if (_page == Page::PostProcess && activeScene)
		{
			const ToneMappingSettings& toneMapping = activeScene->GetToneMappingSettings();
			const BloomSettings& bloom = renderer.GetBloomSettings();
			const SSAOSettings& ssao = renderer.GetSSAOSettings();
			const ScreenSpaceOutlineSettings& outline = renderer.GetScreenSpaceOutlineSettings();
			text << L"\nSelect [Up / Down], Adjust [Left / Right], Coarse [Shift], Toggle [Enter], Print [P]";
			auto appendValue = [this, &text](PostProcessSetting setting, const wchar_t* name, float value, int32 precision)
			{
				text << L'\n' << (_postProcessSettingIndex == static_cast<uint32>(setting) ? L"> " : L"  ") << name << L" : " << std::setprecision(precision) << value;
			};
			auto appendState = [this, &text](PostProcessSetting setting, const wchar_t* name, bool enabled)
			{
				text << L'\n' << (_postProcessSettingIndex == static_cast<uint32>(setting) ? L"> " : L"  ") << name << L" : " << (enabled ? L"On" : L"Off");
			};
			text << L"\n[Tone Mapping]";
			appendValue(PostProcessSetting::Exposure, L"Exposure", toneMapping.exposure, 2);
			text << L"\n[Bloom]";
			appendValue(PostProcessSetting::BloomThreshold, L"Threshold", bloom.threshold, 2);
			appendValue(PostProcessSetting::BloomSoftKnee, L"Soft Knee", bloom.softKnee, 2);
			appendValue(PostProcessSetting::BloomIntensity, L"Intensity", bloom.intensity, 2);
			text << L"\n[SSAO]";
			appendValue(PostProcessSetting::SSAORadius, L"Radius", ssao.radius, 3);
			appendValue(PostProcessSetting::SSAOFadeStart, L"Fade Start", ssao.fadeStart, 3);
			appendValue(PostProcessSetting::SSAOFadeEnd, L"Fade End", ssao.fadeEnd, 3);
			appendValue(PostProcessSetting::SSAOSurfaceEpsilon, L"Surface Epsilon", ssao.surfaceEpsilon, 4);
			appendValue(PostProcessSetting::SSAOStrength, L"Strength", ssao.strength, 2);
			text << L"\n[Outline]";
			appendState(PostProcessSetting::OutlineEnabled, L"Enabled", renderer.IsScreenSpaceOutlineEnabled());
			appendValue(PostProcessSetting::OutlineDepthThreshold, L"Depth Threshold", outline.depthThreshold, 4);
			appendValue(PostProcessSetting::OutlineNormalThreshold, L"Normal Threshold", outline.normalThreshold, 2);
			appendValue(PostProcessSetting::OutlineThickness, L"Thickness", outline.thickness, 2);
			appendValue(PostProcessSetting::OutlineColorR, L"Color R", outline.color.x, 2);
			appendValue(PostProcessSetting::OutlineColorG, L"Color G", outline.color.y, 2);
			appendValue(PostProcessSetting::OutlineColorB, L"Color B", outline.color.z, 2);
			appendValue(PostProcessSetting::OutlineOpacity, L"Opacity", outline.color.w, 2);
			text << L"\n[Anti-Aliasing]";
			appendState(PostProcessSetting::FXAAEnabled, L"FXAA Enabled", renderer.IsFXAAEnabled());
		}
		else if (_page == Page::Material && activeScene)
		{
			GameObject* selectedObject = _selectedMaterialObject.Get();
			const BoundingVolume selectedBounds = selectedObject ? GetMeshWorldBounds(*selectedObject) : BoundingVolume{};
			if (selectedBounds.isValid)
				APPLICATION.GetDebugRenderer().RequestDrawBox(selectedBounds.box, Colors::Yellow);
			text << L"\nSelect Mesh at Mouse Cursor [Middle Click], Cycle Overlapping Meshes [Repeat]";
			text << L"\nSelect Material Slot [Up / Down], Adjust Emissive [Left / Right, 0.05], Coarse [Shift, 0.5]";
			text << L"\nSelected Object Bounds: Yellow, Selected Material Slot: Magenta Tint";
			if (selectedObject)
			{
				const uint32 materialSlotCount = GetMaterialSlotCount(*selectedObject);
				if (_selectedEnvironmentModelIndex)
					text << L"\nSelected Mesh : Environment Model " << *_selectedEnvironmentModelIndex;
				else if (selectedObject->GetComponent<QamilStateMachineComponent>())
					text << L"\nSelected Mesh : Qamil";
				else if (selectedObject->GetComponent<SkeletalMeshComponent>())
					text << L"\nSelected Mesh : Skeletal Mesh";
				else
					text << L"\nSelected Mesh : Static Mesh";
				if (materialSlotCount > 0)
				{
					_materialSlotIndex = std::min(_materialSlotIndex, materialSlotCount - 1);
					const MeshMaterialSlot* materialSlot = GetMaterialSlot(*selectedObject, _materialSlotIndex);
					const Material* material = GetMaterial(*selectedObject, _materialSlotIndex);
					text << L"\nMaterial Slot : " << _materialSlotIndex << L" / " << materialSlotCount - 1;
					text << L"\nBaseColor Texture : " << (materialSlot ? materialSlot->textureKeys[static_cast<uint32>(TextureSlot::BaseColor)] : std::wstring{});
					text << L"\nEmissive Intensity : " << std::setprecision(2) << (material ? material->GetEmissiveIntensity() : 0.f);
				}
			}
			else
				text << L"\nSelected Mesh : None";
		}
		else if (_page == Page::Audio)
		{
			const std::span<const RhythmBGMDesc* const> bgms = GetRhythmBGMs();
			text << L"\nBGM Change [Up / Down], Restart [Enter], Offset [[ / ]], Coarse [Shift], Print [P]";
			text << L"\nTap Beat [Left / Right Mouse]";
			for (uint32 index = 0; index < bgms.size() && index < _audioSongOffsets.size(); ++index)
			{
				text << L'\n' << (_audioBGMIndex == index ? L"> " : L"  ") << bgms[index]->commonResourceKey << L" : " << std::showpos << std::setprecision(3) << _audioSongOffsets[index] << std::noshowpos << L" s";
				if (GetCurrentRhythmBGM() == bgms[index])
					text << L" [Playing]";
			}
			if (_beatSystem.HasPlaybackTime())
				text << L"\nCurrent Beat : " << std::setprecision(3) << _beatSystem.GetCurrentBeat();
			else
				text << L"\nCurrent Beat : Not Playing";
			const float averageError = _audioInputSampleCount > 0 ? _audioInputErrorSum / static_cast<float>(_audioInputSampleCount) : 0.f;
			text << L"\nAverage Error : " << std::showpos << std::setprecision(1) << averageError * 1000.f << std::noshowpos << L" ms (" << _audioInputSampleCount << L" samples)";
			text << L"\nRecommendation : " << (_audioInputSampleCount > 0 ? GetAudioOffsetRecommendation(averageError) : L"정박에 맞춰 여러 번 클릭해주세요");
		}

		textBlock->SetText(text.str());
	}
}

#endif
