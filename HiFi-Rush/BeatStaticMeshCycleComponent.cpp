#include "BeatStaticMeshCycleComponent.h"
#include "BeatSystem.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"

#include <algorithm>
#include <utility>

namespace gm
{
	BeatStaticMeshCycleComponent::BeatStaticMeshCycleComponent(const BeatSystem& beatSystem, StaticMeshComponent& meshComponent, std::vector<std::shared_ptr<StaticMesh>> meshVariants)
		: _beatSystem(beatSystem), _meshComponent(meshComponent), _meshVariants(std::move(meshVariants))
	{}

	void BeatStaticMeshCycleComponent::OnInitialize()
	{
		GM_ASSERT_RETURN(_meshVariants.empty() == false, "BeatStaticMeshCycleComponent는 하나 이상의 StaticMesh 변형이 필요합니다.");

		const std::shared_ptr<StaticMesh>& currentMesh = _meshComponent.GetStaticMesh();
		const auto currentVariant = std::find(_meshVariants.begin(), _meshVariants.end(), currentMesh);
		if (currentVariant == _meshVariants.end())
		{
			_meshComponent.SetStaticMesh(_meshVariants.front());
			_currentVariantIndex = 0;
			return;
		}

		_currentVariantIndex = static_cast<uint32>(std::distance(_meshVariants.begin(), currentVariant));
	}

	void BeatStaticMeshCycleComponent::OnTick(float)
	{
		if (_beatSystem.DidCrossBeatBoundary() == false || _meshVariants.size() <= 1)
			return;

		_currentVariantIndex = (_currentVariantIndex + 1) % static_cast<uint32>(_meshVariants.size());
		_meshComponent.SetStaticMesh(_meshVariants[_currentVariantIndex]);
	}
}
