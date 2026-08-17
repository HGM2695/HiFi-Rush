#include "RhythmBarWidget.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "CanvasPanel.h"
#include "Image.h"

#include <algorithm>
#include <string>

namespace gm
{
	namespace
	{
		constexpr wchar_t CatTextureKey[] = L"T_Rhytm_Meter_Cat_Black";
		constexpr wchar_t BackCircleTextureKey[] = L"T_Rhytm_Meter_halftone_bg";
		constexpr wchar_t BigLineTextureKey[] = L"T_Rhytm_Meter_BGline";
		constexpr wchar_t SmallLineTextureKey[] = L"T_Rhytm_Meter_BGline_Small";
		constexpr wchar_t LeftMarkerTextureKey[] = L"Left_Half_Circle";
		constexpr wchar_t RightMarkerTextureKey[] = L"Right_Half_Circle";
		constexpr Vector2 RhythmBarCenter{ 800.f, 795.f };

		constexpr Vector2 BigLineSize{ 1150.f, 150.f };
		constexpr Vector2 SmallLineMinSize{ 575.f, 60.f };
		constexpr float SmallLineMaxWidth = 862.5f;
		constexpr float BackCircleMinSize = 100.f;
		constexpr float BackCircleMaxSize = 150.f;
		constexpr float CatMinSize = 62.5f;
		constexpr float CatMaxSize = 93.75f;

		constexpr Vector2 BeatMarkerSize{ 37.5f, 62.5f };
		constexpr float LeftBeatMarkerStartX = 336.25f;
		constexpr float RightBeatMarkerStartX = 1265.f;
		constexpr float BeatMarkerMoveDistance = 150.f;
		constexpr float BeatMarkerLifetime = 3.f;

		float Lerp(float from, float to, float ratio)
		{
			return from + (to - from) * ratio;
		}

		void SetLinearSampler(Image& image)
		{
			SamplerDesc samplerDesc{};
			samplerDesc.filter = TextureFilter::Linear;
			image.SetSamplerDesc(samplerDesc);
		}
	}

	RhythmBarWidget::RhythmBarWidget(const BeatSystem& beatSystem)
		: _beatSystem(beatSystem)
	{}

	std::unique_ptr<Widget> RhythmBarWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);

		_bigLineImage = root->AddNamedChild<Image>(L"BigLine", BigLineTextureKey);
		_smallLineImage = root->AddNamedChild<Image>(L"SmallLine", SmallLineTextureKey);
		_backCircleImage = root->AddNamedChild<Image>(L"BackCircle", BackCircleTextureKey);
		_catImage = root->AddNamedChild<Image>(L"Cat", CatTextureKey);

		SetLinearSampler(*_bigLineImage);
		SetLinearSampler(*_smallLineImage);
		SetLinearSampler(*_backCircleImage);
		SetLinearSampler(*_catImage);

		for (uint32 markerIndex = 0; markerIndex < BeatMarkerPairCount; ++markerIndex)
		{
			BeatMarkerPair& markerPair = _beatMarkerPairs[markerIndex];
			markerPair.left = root->AddNamedChild<Image>(L"LeftMarker" + std::to_wstring(markerIndex), LeftMarkerTextureKey);
			markerPair.right = root->AddNamedChild<Image>(L"RightMarker" + std::to_wstring(markerIndex), RightMarkerTextureKey);
			SetLinearSampler(*markerPair.left);
			SetLinearSampler(*markerPair.right);
			markerPair.left->SetVisible(false);
			markerPair.right->SetVisible(false);
		}

		UpdateBaseImages(0.f);

		return root;
	}

	void RhythmBarWidget::OnTick(float)
	{
		const float pulse = _beatSystem.HasPlaybackTime() ? BeatMath::EvaluateBeatIntervalPulse(_beatSystem.GetCurrentBeat(), 1.f) : 0.f;

		UpdateBaseImages(pulse);
		UpdateBeatMarkers();
	}

	void RhythmBarWidget::UpdateBaseImages(float pulse)
	{
		const Vector2 center{ RhythmBarCenter.x, RhythmBarCenter.y - _verticalOffset };
		const float smallLineWidth = Lerp(SmallLineMinSize.x, SmallLineMaxWidth, pulse);
		const float backCircleSize = Lerp(BackCircleMinSize, BackCircleMaxSize, pulse);
		const float catSize = Lerp(CatMinSize, CatMaxSize, pulse);

		ApplyImageGeometry(*_bigLineImage, center, BigLineSize);
		ApplyImageGeometry(*_smallLineImage, center, Vector2{ smallLineWidth, SmallLineMinSize.y });
		ApplyImageGeometry(*_backCircleImage, center, Vector2{ backCircleSize, backCircleSize });
		ApplyImageGeometry(*_catImage, center, Vector2{ catSize, catSize });
		_catImage->SetColorBlend(Colors::Red, 1.f - pulse);
	}

	void RhythmBarWidget::UpdateBeatMarkers()
	{
		if (_beatSystem.HasPlaybackTime() == false)
		{
			ResetBeatMarkers();
			return;
		}

		const int64 currentBeatIndex = _beatSystem.GetCurrentBeatIndex();
		if (_isBeatTrackingInitialized == false || currentBeatIndex < _lastBeatIndex)
		{
			ResetBeatMarkers();
			_isBeatTrackingInitialized = true;
		}
		else if (currentBeatIndex > _lastBeatIndex)
		{
			const int64 firstBeatIndex = std::max(_lastBeatIndex + 1, currentBeatIndex - static_cast<int64>(BeatMarkerPairCount) + 1);
			for (int64 beatIndex = firstBeatIndex; beatIndex <= currentBeatIndex; ++beatIndex)
				SpawnBeatMarkerPair(beatIndex);
		}

		_lastBeatIndex = currentBeatIndex;

		const float currentBeat = _beatSystem.GetCurrentBeat();
		for (BeatMarkerPair& markerPair : _beatMarkerPairs)
		{
			if (markerPair.isActive == false)
				continue;

			const float ageBeats = currentBeat - static_cast<float>(markerPair.spawnBeatIndex);
			if (ageBeats < 0.f || ageBeats >= BeatMarkerLifetime)
			{
				markerPair.isActive = false;
				markerPair.left->SetVisible(false);
				markerPair.right->SetVisible(false);
				continue;
			}

			const float markerY = RhythmBarCenter.y - _verticalOffset;
			const float leftX = LeftBeatMarkerStartX + BeatMarkerMoveDistance * ageBeats;
			const float rightX = RightBeatMarkerStartX - BeatMarkerMoveDistance * ageBeats;
			const float opacity = std::clamp(ageBeats, 0.f, 1.f);

			ApplyImageGeometry(*markerPair.left, Vector2{ leftX, markerY }, BeatMarkerSize);
			ApplyImageGeometry(*markerPair.right, Vector2{ rightX, markerY }, BeatMarkerSize);
			markerPair.left->SetOpacity(opacity);
			markerPair.right->SetOpacity(opacity);
		}
	}

	void RhythmBarWidget::SpawnBeatMarkerPair(int64 beatIndex)
	{
		BeatMarkerPair& markerPair = _beatMarkerPairs[_nextBeatMarkerPairIndex];
		markerPair.spawnBeatIndex = beatIndex;
		markerPair.isActive = true;
		markerPair.left->SetVisible(true);
		markerPair.right->SetVisible(true);
		markerPair.left->SetOpacity(0.f);
		markerPair.right->SetOpacity(0.f);

		_nextBeatMarkerPairIndex = (_nextBeatMarkerPairIndex + 1) % BeatMarkerPairCount;
	}

	void RhythmBarWidget::ResetBeatMarkers()
	{
		for (BeatMarkerPair& markerPair : _beatMarkerPairs)
		{
			markerPair.isActive = false;
			markerPair.left->SetVisible(false);
			markerPair.right->SetVisible(false);
		}

		_nextBeatMarkerPairIndex = 0;
		_isBeatTrackingInitialized = false;
	}

	void RhythmBarWidget::ApplyImageGeometry(Image& image, const Vector2& center, const Vector2& size) const
	{
		image.SetPosition(center);
		image.SetSize(size);
	}
}
