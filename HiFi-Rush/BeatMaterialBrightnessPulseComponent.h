#pragma once

#include "Component.h"
#include "MathTypes.h"

namespace gm
{
	class BeatSystem;
	class Material;

	struct BeatMaterialBrightnessPulseDesc
	{
		uint32	materialSlot = 0;
		uint32	activeBeatMask = 1;
		uint32	patternLengthBeats = 1;
		float	minimumBrightness = 1.f;
		float	maximumBrightness = 1.f;
	};

	class BeatMaterialBrightnessPulseComponent final : public Component
	{
	public:
		BeatMaterialBrightnessPulseComponent(const BeatSystem& beatSystem, const BeatMaterialBrightnessPulseDesc& desc);

	protected:
		void OnInitialize() override;
		void OnTick(float deltaTime) override;

	private:
		bool ResolveMaterial();
		void UpdateBrightness();

	private:
		const BeatSystem&				_beatSystem;
		BeatMaterialBrightnessPulseDesc	_desc{};
		Material*						_material = nullptr;
		Color							_initialColorMultiplier = Colors::White;
	};
}
