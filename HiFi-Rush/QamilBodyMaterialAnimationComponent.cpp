#include "QamilBodyMaterialAnimationComponent.h"

#include "BeatMath.h"
#include "BeatSystem.h"
#include "BuiltinGraphicsResources.h"
#include "GameObject.h"
#include "Material.h"
#include "QamilResources.h"
#include "QamilStateMachineComponent.h"
#include "QamilStateTypes.h"
#include "Random.h"
#include "Resources.h"
#include "Shader.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"

#include <algorithm>

namespace gm
{
	namespace
	{
		constexpr float QamilPhaseDefaultEmissiveIntensity = 4.5f;
		constexpr float QamilPhase3EmissiveIntensity = 7.f;

		Color GetQamilPhaseColor(QamilPhase phase)
		{
			return phase >= QamilPhase::Phase3 ? Colors::Red : Colors::Yellow;
		}

		struct alignas(16) UVBarGraphConstantPS
		{
			Vector4 barHeights{};
			Color	barColor = Colors::Yellow;
			float	lineInterval = 0.01f;
			float	lineThickness = 0.001f;
			float	opacity = 0.2f;
			float	emissiveIntensity = 15.f;
		};

		static_assert(sizeof(UVBarGraphConstantPS) == 48);
	}

	QamilBodyMaterialAnimationComponent::QamilBodyMaterialAnimationComponent(Resources& resources, const BeatSystem& beatSystem)
		: _resources(resources), _beatSystem(beatSystem)
	{}

	void QamilBodyMaterialAnimationComponent::OnInitialize()
	{
		SkeletalMeshComponent* meshComponent = GetOwner().GetComponent<SkeletalMeshComponent>();
		GM_ASSERT_RETURN(meshComponent, "QamilBodyMaterialAnimationComponent는 SkeletalMeshComponent가 필요합니다.");
		_stateMachine = GetOwner().GetComponent<QamilStateMachineComponent>();
		GM_ASSERT_RETURN(_stateMachine, "QamilBodyMaterialAnimationComponent는 QamilStateMachineComponent가 필요합니다.");
		_barGraphMaterial = ResolveSectionMaterial(*meshComponent, QamilBarGraphSectionIndex);
		_corePulseMaterial = ResolveSectionMaterial(*meshComponent, QamilCorePulseSectionIndex);
		GM_ASSERT_RETURN(_barGraphMaterial && _corePulseMaterial, "Qamil Body Material Animation 대상 Section Material을 찾을 수 없습니다.");
		for (uint32 index = 0; index < _phaseEmissiveMaterials.size(); ++index)
		{
			_phaseEmissiveMaterials[index] = meshComponent->GetMaterial(QamilPhaseEmissiveMaterialSlotIndices[index]);
			GM_ASSERT_RETURN(_phaseEmissiveMaterials[index], "Qamil Phase Emissive 대상 Material Slot을 찾을 수 없습니다. slot=%u", QamilPhaseEmissiveMaterialSlotIndices[index]);
		}

		const std::shared_ptr<Shader> barGraphShader = _resources.Find<Shader>(BuiltinResourceKey::UVBarGraphPS);
		GM_ASSERT_RETURN(barGraphShader, "UV Bar Graph Pixel Shader가 로드되지 않았습니다.");
		_barGraphMaterial->SetPixelShader(barGraphShader);
		_barGraphMaterial->SetSurfaceMode(SurfaceMode::Transparent);
		_barGraphMaterial->SetShadingModel(ShadingModel::Unlit);
		_barGraphMaterial->SetOutlineMode(OutlineMode::Disabled);
		RasterizerDesc rasterizerDesc = _barGraphMaterial->GetRasterizerDesc();
		rasterizerDesc.cullMode = CullMode::None;
		_barGraphMaterial->SetRasterizerDesc(rasterizerDesc);
		_initialCoreColorMultiplier = _corePulseMaterial->GetColorData().colorMultiplier;
		UpdateBarGraph();
		UpdatePhaseEmissiveMaterials();
		UpdateCorePulse();
	}

	void QamilBodyMaterialAnimationComponent::OnTick(float)
	{
		if (_barGraphMaterial == nullptr || _corePulseMaterial == nullptr || _stateMachine == nullptr)
			return;

		if (_beatSystem.HasPlaybackTime())
		{
			const int64 currentBeatIndex = _beatSystem.GetCurrentBeatIndex();
			if (currentBeatIndex < _lastBarShuffleBeat || currentBeatIndex > _lastBarShuffleBeat + 1)
			{
				ShuffleBarHeights();
				_lastBarShuffleBeat = currentBeatIndex;
			}
		}
		UpdateBarGraph();
		UpdatePhaseEmissiveMaterials();
		UpdateCorePulse();
	}

	Material* QamilBodyMaterialAnimationComponent::ResolveSectionMaterial(const SkeletalMeshComponent& meshComponent, uint32 sectionIndex) const
	{
		const std::shared_ptr<SkeletalMesh>& skeletalMesh = meshComponent.GetSkeletalMesh();
		if (skeletalMesh == nullptr)
			return nullptr;
		const MeshSection* section = skeletalMesh->GetSection(sectionIndex);
		return section ? meshComponent.GetMaterial(section->materialSlotIndex) : nullptr;
	}

	void QamilBodyMaterialAnimationComponent::ShuffleBarHeights()
	{
		for (int32 index = static_cast<int32>(_baseBarHeights.size()) - 1; index > 0; --index)
			std::swap(_baseBarHeights[index], _baseBarHeights[Math::RandomInt(0, index)]);
	}

	void QamilBodyMaterialAnimationComponent::UpdateBarGraph()
	{
		if (_barGraphMaterial == nullptr)
			return;

		const float currentBeat = _beatSystem.HasPlaybackTime() ? _beatSystem.GetCurrentBeat() : 0.f;
		const float pulse = BeatMath::EvaluateBeatIntervalPulse(currentBeat, 1.f);
		const float heightOffset = _beatSystem.GetMusicPeak() * 0.4f + pulse * 0.05f;
		UVBarGraphConstantPS constant{};
		constant.barHeights = Vector4{ std::clamp(_baseBarHeights[0] + heightOffset, 0.f, 1.f), std::clamp(_baseBarHeights[1] + heightOffset, 0.f, 1.f), std::clamp(_baseBarHeights[2] + heightOffset, 0.f, 1.f), std::clamp(_baseBarHeights[3] + heightOffset, 0.f, 1.f) };
		const Color phaseColor = GetQamilPhaseColor(_stateMachine->GetCurrentPhase());
		constant.barColor = ConvertSRGBToLinear(phaseColor);
		_barGraphMaterial->SetConstantData(ShaderStage::Pixel, 2, constant);
	}

	void QamilBodyMaterialAnimationComponent::UpdatePhaseEmissiveMaterials()
	{
		const QamilPhase phase = _stateMachine->GetCurrentPhase();
		if (_appliedEmissivePhase == phase)
			return;

		const Color phaseColor = GetQamilPhaseColor(phase);
		const float emissiveIntensity = phase >= QamilPhase::Phase3 ? QamilPhase3EmissiveIntensity : QamilPhaseDefaultEmissiveIntensity;
		for (Material* material : _phaseEmissiveMaterials)
		{
			material->SetColorBlend(phaseColor, 1.f);
			material->SetEmissiveColor(phaseColor);
			material->SetEmissiveIntensity(emissiveIntensity);
		}
		_appliedEmissivePhase = phase;
	}

	void QamilBodyMaterialAnimationComponent::UpdateCorePulse()
	{
		if (_corePulseMaterial == nullptr)
			return;

		const float currentBeat = _beatSystem.HasPlaybackTime() ? _beatSystem.GetCurrentBeat() : 0.f;
		const float brightness = BeatMath::EvaluateBeatIntervalPulse(currentBeat, 1.f) * 2.f + 0.5f;
		_corePulseMaterial->SetColorMultiplier(Color{ _initialCoreColorMultiplier.x * brightness, _initialCoreColorMultiplier.y * brightness, _initialCoreColorMultiplier.z * brightness, _initialCoreColorMultiplier.w });
	}
}
