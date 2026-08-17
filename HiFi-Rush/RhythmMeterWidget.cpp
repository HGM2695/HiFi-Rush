#include "RhythmMeterWidget.h"

#include "BeatMath.h"
#include "BeatSystem.h"
#include "CanvasPanel.h"
#include "Image.h"
#include "RhythmRankComponent.h"

#include <array>

namespace gm
{
	namespace
	{
		struct RankVisualInfo
		{
			const wchar_t*	gaugeTextureKey = nullptr;
			const wchar_t*	gradeMarkTextureKey = nullptr;
			const wchar_t*	shadowTextureKey = nullptr;
			Vector2			gaugeCenter{};
			Vector2			gaugeSize{};
			Vector2			gradeMarkCenter{};
			Vector2			gradeMarkSize{};
			Vector2			shadowCenter{};
			Vector2			shadowSize{};
			float			radialStartAngle = 0.f;
			float			radialSweepAngle = 0.f;
		};

		constexpr std::array<RankVisualInfo, RhythmRankCount> RankVisualInfos =
		{{
			{ L"C_Gauge", L"C_GradeMark", L"C_Shadow", { 1435.625f, 169.375f }, { 201.25f, 206.25f }, { 1425.625f, 164.375f }, { 116.25f, 153.75f }, { 1435.625f, 174.375f }, { 206.25f, 218.75f }, 1.29695429f, 6.00717416f },
			{ L"B_Gauge", L"B_GradeMark", L"B_Shadow", { 1435.625f, 167.5f }, { 203.75f, 215.f }, { 1430.f, 163.125f }, { 132.5f, 131.25f }, { 1435.625f, 173.75f }, { 208.75f, 227.5f }, 1.35970299f, 5.98247552f },
			{ L"A_Gauge", L"A_GradeMark", L"A_Shadow", { 1435.f, 167.5f }, { 205.f, 215.f }, { 1431.25f, 165.625f }, { 132.5f, 151.25f }, { 1435.625f, 173.75f }, { 208.75f, 227.5f }, 1.32581766f, 6.01168097f },
			{ L"S_Gauge", L"S_GradeMark", L"A_Shadow", { 1435.f, 167.5f }, { 205.f, 215.f }, { 1440.f, 163.75f }, { 132.5f, 150.f }, { 1435.625f, 173.75f }, { 208.75f, 227.5f }, 1.28606589f, 6.03122757f },
		}};

		constexpr Vector2 BlackKeysCenter{ 1430.625f, 141.875f };
		constexpr Vector2 BlackKeysSize{ 238.75f, 183.75f };
		constexpr Vector2 BInnerSpikeCenter{ 1430.f, 165.f };
		constexpr Vector2 BInnerSpikeSize{ 130.f, 127.5f };
		constexpr Vector2 BOuterSpikeCenter{ 1430.f, 160.f };
		constexpr Vector2 BOuterSpikeSize{ 235.f, 242.5f };
		constexpr Vector2 RadialEffectCenter{ 1431.25f, 178.125f };
		constexpr Vector2 RadialEffectSize{ 365.f, 336.25f };
		constexpr Vector2 AInnerSpikeCenter{ 1428.75f, 163.125f };
		constexpr Vector2 AInnerSpikeSize{ 127.5f, 123.75f };
		constexpr Vector2 AOuterSpikeCenter{ 1419.375f, 160.f };
		constexpr Vector2 AOuterSpikeSize{ 261.25f, 262.5f };
		constexpr Vector2 BigRadialEffectCenter{ 1428.75f, 175.625f };
		constexpr Vector2 BigRadialEffectSize{ 410.f, 403.75f };
		constexpr Vector2 SparkCenter{ 1420.625f, 165.625f };
		constexpr Vector2 SparkSize{ 343.75f, 313.75f };
		constexpr Vector2 StarCenter{ 1390.625f, 191.25f };
		constexpr Vector2 StarSize{ 338.75f, 370.f };
		constexpr Vector2 ThunderCenter{ 1425.f, 150.625f };
		constexpr Vector2 ThunderSize{ 277.5f, 278.75f };

		float Lerp(float from, float to, float ratio)
		{
			return from + (to - from) * ratio;
		}
	}

	RhythmMeterWidget::RhythmMeterWidget(const BeatSystem& beatSystem, RhythmRankComponent& rhythmRankComponent)
		: _beatSystem(beatSystem), _rhythmRankComponent(rhythmRankComponent)
	{
	}

	std::unique_ptr<Widget> RhythmMeterWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);

		_shadowImage = root->AddNamedChild<Image>(L"Shadow", L"C_Shadow");
		_radialEffectImage = root->AddNamedChild<Image>(L"RadialEffect", L"B_RadialEffect");
		_bigRadialEffectImage = root->AddNamedChild<Image>(L"BigRadialEffect", L"A_BigRadialEffect");
		_starImage = root->AddNamedChild<Image>(L"Star", L"S_Star");
		_outerSpikeImage = root->AddNamedChild<Image>(L"OuterSpike", L"B_OuterSpike");
		_innerSpikeImage = root->AddNamedChild<Image>(L"InnerSpike", L"B_InnerSpike");
		_blackKeysImage = root->AddNamedChild<Image>(L"BlackKeys", L"C_BlackKeys");
		_gaugeBackgroundImage = root->AddNamedChild<Image>(L"GaugeBackground", L"C_Gauge");
		_gaugeFillImage = root->AddNamedChild<Image>(L"GaugeFill", L"C_Gauge");
		_gradeMarkImage = root->AddNamedChild<Image>(L"GradeMark", L"C_GradeMark");
		_sparkImage = root->AddNamedChild<Image>(L"Spark", L"A_Spark");
		_thunderImage = root->AddNamedChild<Image>(L"Thunder", L"S_Thunder");

		_gaugeBackgroundImage->SetColorBlend(Colors::Black, 0.5f);
		UpdateRankAppearance();
		return root;
	}

	void RhythmMeterWidget::OnInitialize()
	{
		_rhythmRankComponent.OnRhythmRankChanged.Subscribe(_rhythmRankChangedConnection,
			[this](const RhythmRankChangedEvent& event)
			{
				HandleRhythmRankChanged(event);
			});

		_gaugeFillImage->SetFillRatio(_rhythmRankComponent.GetGaugeRatio());
	}

	void RhythmMeterWidget::OnTick(float)
	{
		UpdateBeatAnimation();
	}

	void RhythmMeterWidget::HandleRhythmRankChanged(const RhythmRankChangedEvent& event)
	{
		if (event.previousRank != event.currentRank)
			UpdateRankAppearance();

		_gaugeFillImage->SetFillRatio(event.gaugeRatio);
	}

	void RhythmMeterWidget::UpdateRankAppearance()
	{
		const RhythmRank rank = _rhythmRankComponent.GetRank();
		const RankVisualInfo& info = RankVisualInfos[GetRhythmRankIndex(rank)];

		_shadowImage->SetTexture(info.shadowTextureKey);
		_gaugeBackgroundImage->SetTexture(info.gaugeTextureKey);
		_gaugeFillImage->SetTexture(info.gaugeTextureKey);
		_gaugeFillImage->SetRadialFill(info.radialStartAngle, info.radialSweepAngle);
		_gradeMarkImage->SetTexture(info.gradeMarkTextureKey);

		SetImageGeometry(*_shadowImage, info.shadowCenter, info.shadowSize);
		SetImageGeometry(*_gaugeBackgroundImage, info.gaugeCenter, info.gaugeSize);
		SetImageGeometry(*_gaugeFillImage, info.gaugeCenter, info.gaugeSize);
		SetImageGeometry(*_gradeMarkImage, info.gradeMarkCenter, info.gradeMarkSize);

		const bool isAtLeastB = rank >= RhythmRank::B;
		const bool isAtLeastA = rank >= RhythmRank::A;
		const bool isS = rank == RhythmRank::S;
		_radialEffectImage->SetVisible(isAtLeastB);
		_outerSpikeImage->SetVisible(isAtLeastB);
		_innerSpikeImage->SetVisible(isAtLeastB);
		_bigRadialEffectImage->SetVisible(isAtLeastA);
		_sparkImage->SetVisible(isAtLeastA);
		_starImage->SetVisible(isS);
		_thunderImage->SetVisible(isS);

		if (isAtLeastA)
		{
			_outerSpikeImage->SetTexture(L"A_OuterSpike");
			_innerSpikeImage->SetTexture(L"A_InnerSpike");
			SetImageGeometry(*_outerSpikeImage, AOuterSpikeCenter, AOuterSpikeSize);
			SetImageGeometry(*_innerSpikeImage, AInnerSpikeCenter, AInnerSpikeSize);
		}
		else
		{
			_outerSpikeImage->SetTexture(L"B_OuterSpike");
			_innerSpikeImage->SetTexture(L"B_InnerSpike");
			SetImageGeometry(*_outerSpikeImage, BOuterSpikeCenter, BOuterSpikeSize);
			SetImageGeometry(*_innerSpikeImage, BInnerSpikeCenter, BInnerSpikeSize);
		}

		SetImageGeometry(*_blackKeysImage, BlackKeysCenter, BlackKeysSize);
		SetImageGeometry(*_radialEffectImage, RadialEffectCenter, RadialEffectSize);
		SetImageGeometry(*_bigRadialEffectImage, BigRadialEffectCenter, BigRadialEffectSize);
		SetImageGeometry(*_sparkImage, SparkCenter, SparkSize);
		SetImageGeometry(*_starImage, StarCenter, StarSize);
		SetImageGeometry(*_thunderImage, ThunderCenter, ThunderSize);
	}

	void RhythmMeterWidget::UpdateBeatAnimation()
	{
		const float pulse = _beatSystem.HasPlaybackTime() ? BeatMath::EvaluateBeatIntervalPulse(_beatSystem.GetCurrentBeat(), 1.f) : 0.f;
		const RankVisualInfo& info = RankVisualInfos[GetRhythmRankIndex(_rhythmRankComponent.GetRank())];
		SetScaledImageGeometry(*_shadowImage, info.shadowCenter, info.shadowSize, Lerp(0.8f, 1.1f, pulse));
		SetScaledImageGeometry(*_blackKeysImage, BlackKeysCenter, BlackKeysSize, Lerp(0.8f, 1.1f, pulse));

		const RhythmRank rank = _rhythmRankComponent.GetRank();
		if (rank >= RhythmRank::B)
		{
			const bool usesARankSpike = rank >= RhythmRank::A;
			SetScaledImageGeometry(*_innerSpikeImage, usesARankSpike ? AInnerSpikeCenter : BInnerSpikeCenter, usesARankSpike ? AInnerSpikeSize : BInnerSpikeSize, Lerp(0.9f, 1.1f, pulse));
			SetScaledImageGeometry(*_outerSpikeImage, usesARankSpike ? AOuterSpikeCenter : BOuterSpikeCenter, usesARankSpike ? AOuterSpikeSize : BOuterSpikeSize, Lerp(0.9f, 1.1f, pulse));
			SetScaledImageGeometry(*_radialEffectImage, RadialEffectCenter, RadialEffectSize, Lerp(0.8f, 1.1f, pulse));
		}

		if (rank >= RhythmRank::A)
		{
			SetScaledImageGeometry(*_bigRadialEffectImage, BigRadialEffectCenter, BigRadialEffectSize, pulse);
			SetScaledImageGeometry(*_sparkImage, SparkCenter, SparkSize, pulse);
		}

		if (rank == RhythmRank::S)
		{
			SetScaledImageGeometry(*_starImage, StarCenter, StarSize, Lerp(0.5f, 1.1f, pulse));
			SetScaledImageGeometry(*_thunderImage, ThunderCenter, ThunderSize, pulse);
		}
	}

	void RhythmMeterWidget::SetImageGeometry(Image& image, const Vector2& center, const Vector2& size) const
	{
		image.SetPosition(center);
		image.SetSize(size);
	}

	void RhythmMeterWidget::SetScaledImageGeometry(Image& image, const Vector2& center, const Vector2& size, float scale) const
	{
		SetImageGeometry(image, center, size * scale);
	}
}
