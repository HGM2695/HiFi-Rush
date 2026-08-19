#pragma once

#include "Event.h"
#include "RhythmRankTypes.h"
#include "UserWidget.h"

namespace gm
{
	class BeatSystem;
	class Image;
	class RhythmRankComponent;
	struct RhythmRankChangedEvent;

	class RhythmMeterWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"RhythmMeterRoot";

		RhythmMeterWidget(const BeatSystem& beatSystem, RhythmRankComponent& rhythmRankComponent);

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnInitialize() override;
		void					OnTick(float deltaTime) override;

	private:
		void					HandleRhythmRankChanged(const RhythmRankChangedEvent& event);
		void					UpdateRankAppearance();
		void					UpdateBeatAnimation();
		void					SetScaledImageGeometry(Image& image, const Vector2& center, const Vector2& size, float scale) const;

		const BeatSystem&		_beatSystem;
		RhythmRankComponent&	_rhythmRankComponent;
		EventConnection			_rhythmRankChangedConnection{};

		Image*					_shadowImage = nullptr;
		Image*					_radialEffectImage = nullptr;
		Image*					_bigRadialEffectImage = nullptr;
		Image*					_starImage = nullptr;
		Image*					_outerSpikeImage = nullptr;
		Image*					_innerSpikeImage = nullptr;
		Image*					_blackKeysImage = nullptr;
		Image*					_gaugeBackgroundImage = nullptr;
		Image*					_gaugeFillImage = nullptr;
		Image*					_gradeMarkImage = nullptr;
		Image*					_sparkImage = nullptr;
		Image*					_thunderImage = nullptr;
	};
}
