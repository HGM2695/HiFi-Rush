#pragma once

#include "Component.h"

#include <memory>
#include <vector>

namespace gm
{
	class BeatSystem;
	class StaticMesh;
	class StaticMeshComponent;

	class BeatStaticMeshCycleComponent : public Component
	{
	public:
		BeatStaticMeshCycleComponent(const BeatSystem& beatSystem, StaticMeshComponent& meshComponent, std::vector<std::shared_ptr<StaticMesh>> meshVariants);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		const BeatSystem&							_beatSystem;
		StaticMeshComponent&						_meshComponent;
		std::vector<std::shared_ptr<StaticMesh>>	_meshVariants;
		uint32										_currentVariantIndex = 0;
	};
}
