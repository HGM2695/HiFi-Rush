#pragma once

#include "BoundingTypes.h"
#include "Component.h"
#include "Types.h"

#include <string>

namespace gm
{
	class SkeletalMeshComponent;
	class StaticMeshComponent;
	class TransformComponent;

	class EnvironmentModelDebugLabelComponent final : public Component
	{
	public:
		explicit EnvironmentModelDebugLabelComponent(uint32 modelIndex);

		static void ToggleLabels();
		static bool AreLabelsVisible() { return _areLabelsVisible; }
		uint32 GetModelIndex() const { return _modelIndex; }

	protected:
		void OnInitialize() override;
		void OnRender() override;

	private:
		BoundingVolume GetWorldBounds() const;

		static inline bool _areLabelsVisible = false;

		uint32				_modelIndex = 0;
		std::wstring			_label;
		TransformComponent*		_transform = nullptr;
		StaticMeshComponent*	_staticMesh = nullptr;
		SkeletalMeshComponent*	_skeletalMesh = nullptr;
	};
}
