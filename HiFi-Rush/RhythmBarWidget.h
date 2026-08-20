#pragma once

#include "UserWidget.h"

#include <array>

	namespace gm
{
	class BeatSystem;
	class Image;

	class RhythmBarWidget final : public UserWidget
	{
	public:
		inline static constexpr const wchar_t* RootWidgetName = L"RhythmBarRoot";

		explicit RhythmBarWidget(const BeatSystem& beatSystem);

		void SetBossLayoutEnabled(bool enabled);

	protected:
		std::unique_ptr<Widget> BuildWidgetTree() override;
		void					OnTick(float deltaTime) override;

	private:
		struct BeatMarkerPair
		{
			Image* left = nullptr;
			Image* right = nullptr;
			int64 spawnBeatIndex = 0;
			bool isActive = false;
		};

		void UpdateBaseImages(float pulse);
		void UpdateBeatMarkers();
		void SpawnBeatMarkerPair(int64 beatIndex);
		void ResetBeatMarkers();

	private:
		static constexpr uint32								BeatMarkerPairCount = 3;
		static constexpr float								BossLayoutVerticalOffset = 55.f;
		static constexpr float								LayoutInterpolationSpeed = 12.f;

		Image*												_bigLineImage = nullptr;
		Image*												_smallLineImage = nullptr;
		Image*												_backCircleImage = nullptr;
		Image*												_catImage = nullptr;

		const BeatSystem&									_beatSystem;
		std::array<BeatMarkerPair, BeatMarkerPairCount>		_beatMarkerPairs{};
		uint32												_nextBeatMarkerPairIndex = 0;
		int64												_lastBeatIndex = 0;
		float												_verticalOffset = 0.f;
		float												_targetVerticalOffset = 0.f;
		bool												_isBeatTrackingInitialized = false;
	};
}
