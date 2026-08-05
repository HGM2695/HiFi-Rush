#pragma once

#include "Component.h"

namespace gm
{
	class BeatSystem;
	class TransformComponent;

	enum class BeatTransformType
	{
		PositionOffset,
		ScaleMultiplier,
	};

	struct BeatTransformDesc
	{
		BeatTransformType	type = BeatTransformType::PositionOffset;
		Vector3				positionOffset{};
		Vector3				maxScaleMultiplier{ 1.f, 1.f, 1.f };
		float				cycleBeats = 1.f;
		float				phaseOffsetBeats = 0.f;
	};

	class BeatTransformComponent : public Component
	{
	public:
		BeatTransformComponent(const BeatSystem& beatSystem, const BeatTransformDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		const BeatSystem&	_beatSystem;
		BeatTransformDesc	_desc{};
		TransformComponent*	_transform = nullptr;
		Vector3				_initialPosition{};
		Vector3				_initialScale{ 1.f, 1.f, 1.f };
	};
}
