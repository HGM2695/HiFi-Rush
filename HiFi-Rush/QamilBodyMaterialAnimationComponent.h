#pragma once

#include "Component.h"
#include "MathTypes.h"
#include "QamilStateTypes.h"

#include <array>

namespace gm
{
	class BeatSystem;
	class Material;
	class QamilStateMachineComponent;
	class Resources;
	class SkeletalMeshComponent;

	class QamilBodyMaterialAnimationComponent final : public Component
	{
	public:
		QamilBodyMaterialAnimationComponent(Resources& resources, const BeatSystem& beatSystem);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		Material* ResolveSectionMaterial(const SkeletalMeshComponent& meshComponent, uint32 sectionIndex) const;
		void ShuffleBarHeights();
		void UpdateBarGraph();
		void UpdatePhaseEmissiveMaterials();
		void UpdateCorePulse();

	private:
		Resources&					_resources;
		const BeatSystem&			_beatSystem;
		QamilStateMachineComponent*	_stateMachine = nullptr;
		Material*					_barGraphMaterial = nullptr;
		Material*					_corePulseMaterial = nullptr;
		std::array<Material*, 2>		_phaseEmissiveMaterials{};
		std::array<float, 4>			_baseBarHeights{ 0.05f, 0.1f, 0.15f, 0.2f };
		Color						_initialCoreColorMultiplier = Colors::White;
		int64						_lastBarShuffleBeat = 0;
		QamilPhase					_appliedEmissivePhase = QamilPhase::Count;
	};
}
