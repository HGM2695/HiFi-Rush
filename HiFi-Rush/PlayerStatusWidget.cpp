#include "PlayerStatusWidget.h"
#include "BeatMath.h"
#include "BeatSystem.h"
#include "CanvasPanel.h"
#include "ChiStateMachineComponent.h"
#include "CombatTypes.h"
#include "HealthComponent.h"
#include "Image.h"
#include "MathUtil.h"
#include "ReverbComponent.h"
#include "RhythmInputJudge.h"

#include <algorithm>
#include <cmath>

namespace gm
{
	namespace
	{
		constexpr wchar_t BackgroundTextureKey[] = L"Background";
		constexpr wchar_t FullBackgroundTextureKey[] = L"Background_Full";
		constexpr wchar_t ShadowTextureKey[] = L"Shadow";
		constexpr wchar_t HealthTextureKey[] = L"T_HealthBar_Bar_9";
		constexpr wchar_t ReverbTextureKey[] = L"Mana_Bar";
		constexpr wchar_t FullReverbTextureKey[] = L"Mana_Bar_Full";
		constexpr wchar_t AfterImageTextureKey[] = L"AfterImage";
		constexpr std::array<const wchar_t*, 9> ThunderTextureKeys =
		{{
			L"Reverve_Full0", L"Reverve_Full1", L"Reverve_Full2",
			L"Reverve_Full3", L"Reverve_Full4", L"Reverve_Full5",
			L"Reverve_Full6", L"Reverve_Full7", L"Reverve_Full8",
		}};

		constexpr Vector2 BackgroundCenter{ 248.75f, 106.875f };
		constexpr Vector2 BackgroundSize{ 437.5f, 153.75f };
		constexpr Vector2 FullBackgroundCenter{ 245.f, 103.75f };
		constexpr Vector2 FullBackgroundSize{ 445.f, 150.f };
		constexpr Vector2 ShadowMoveOffset{ 3.75f, 13.75f };

		constexpr Vector2 HealthCenter{ 290.f, 106.25f };
		constexpr Vector2 HealthSize{ 232.5f, 272.5f };
		constexpr Vector2 ReverbCenter{ 274.375f, 74.375f };
		constexpr Vector2 ReverbSize{ 171.25f, 171.25f };
		constexpr Vector2 FullReverbCenter{ 274.375f, 73.125f };
		constexpr Vector2 ThunderCenter{ 301.25f, 61.25f };
		constexpr Vector2 ThunderSize{ 375.f, 125.f };

		constexpr Vector2 AfterImageCenter{ 264.375f, 108.125f };
		constexpr Vector2 AfterImageSize{ 468.75f, 151.25f };
		constexpr float GaugeRotation = Math::DegreesToRadians(-36.5f);
		constexpr float StatusSmoothingSpeed = 10.f;
		constexpr float FullReverbThreshold = 0.999f;
		constexpr float AfterImageDurationBeats = 0.8f;
	}

	PlayerStatusWidget::PlayerStatusWidget(const BeatSystem& beatSystem, HealthComponent& healthComponent, ReverbComponent& reverbComponent, ChiStateMachineComponent& stateMachine)
		: _beatSystem(beatSystem), _healthComponent(healthComponent), _reverbComponent(reverbComponent), _stateMachine(stateMachine)
	{
		_healthRatio = healthComponent.GetHealthRatio();
		_targetHealthRatio = _healthRatio;
		_reverbRatio = reverbComponent.GetReverbRatio();
		_targetReverbRatio = _reverbRatio;
	}

	void PlayerStatusWidget::PlayBeatFeedback()
	{
		if (_isAfterImageActive || _beatSystem.HasPlaybackTime() == false)
			return;

		_afterImageStartBeat = _beatSystem.GetCurrentBeat();
		_isAfterImageActive = true;
		for (AfterImageEntry& entry : _afterImages)
			entry.image->SetVisible(true);
	}

	std::unique_ptr<Widget> PlayerStatusWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);

		constexpr std::array<Color, AfterImageCount> afterImageColors =
		{
			Colors::Orange,
			ColorFromRGBA(255, 195, 0),
			ColorFromRGBA(255, 225, 0),
			Colors::Yellow,
		};
		constexpr std::array<float, AfterImageCount> afterImageOpacities{ 0.5f, 0.4f, 0.3f, 0.3f };

		for (uint32 index = 0; index < AfterImageCount; ++index)
		{
			AfterImageEntry& entry = _afterImages[index];
			entry.image = root->AddNamedChild<Image>(L"AfterImage" + std::to_wstring(index), AfterImageTextureKey);
			entry.targetOffset = Vector2{ static_cast<float>(index + 2) * 10.f, static_cast<float>(index + 2) * 20.f };
			SetImageGeometry(*entry.image, AfterImageCenter, AfterImageSize);
			entry.image->SetColorBlend(afterImageColors[index], 1.f);
			entry.image->SetOpacity(afterImageOpacities[index]);
			entry.image->SetVisible(false);
		}

		_shadowImage = root->AddNamedChild<Image>(L"Shadow", ShadowTextureKey);
		_backgroundImage = root->AddNamedChild<Image>(L"Background", BackgroundTextureKey);
		_fullBackgroundImage = root->AddNamedChild<Image>(L"FullBackground", FullBackgroundTextureKey);
		_reverbImage = root->AddNamedChild<Image>(L"Reverb", ReverbTextureKey);
		_fullReverbImage = root->AddNamedChild<Image>(L"FullReverb", FullReverbTextureKey);
		_healthImage = root->AddNamedChild<Image>(L"Health", HealthTextureKey);
		_thunderImage = root->AddNamedChild<Image>(L"Thunder", ThunderTextureKeys.front());

		SetImageGeometry(*_shadowImage, BackgroundCenter, BackgroundSize);
		SetImageGeometry(*_backgroundImage, BackgroundCenter, BackgroundSize);
		SetImageGeometry(*_fullBackgroundImage, FullBackgroundCenter, FullBackgroundSize);
		SetImageGeometry(*_reverbImage, ReverbCenter, ReverbSize);
		SetImageGeometry(*_fullReverbImage, FullReverbCenter, ReverbSize);
		SetImageGeometry(*_healthImage, HealthCenter, HealthSize);
		SetImageGeometry(*_thunderImage, ThunderCenter, ThunderSize);

		_reverbImage->SetRotation(GaugeRotation);
		_fullReverbImage->SetRotation(GaugeRotation);
		_healthImage->SetRotation(GaugeRotation);
		_fullBackgroundImage->SetVisible(false);
		_fullReverbImage->SetVisible(false);
		_thunderImage->SetVisible(false);

		return root;
	}

	void PlayerStatusWidget::OnInitialize()
	{
		_healthComponent.OnHealthChanged.Subscribe(_healthChangedConnection,
			[this](const HealthChangedEvent& event)
			{
				HandleHealthChanged(event);
			});
		_reverbComponent.OnReverbChanged.Subscribe(_reverbChangedConnection,
			[this](const ReverbChangedEvent& event)
			{
				HandleReverbChanged(event);
			});
		_stateMachine.OnRhythmActionStarted.Subscribe(_rhythmActionStartedConnection,
			[this](const RhythmJudgeResult& result)
			{
				HandleRhythmActionStarted(result);
			});

		_healthImage->SetFillRatio(_healthRatio);
		_reverbImage->SetFillRatio(_reverbRatio);
	}

	void PlayerStatusWidget::OnTick(float deltaTime)
	{
		UpdateStatus(deltaTime);
		UpdateShadow();
		UpdateThunder();
		UpdateAfterImages();
	}

	void PlayerStatusWidget::HandleHealthChanged(const HealthChangedEvent& event)
	{
		_targetHealthRatio = event.maxHealth > 0 ? static_cast<float>(event.currentHealth) / static_cast<float>(event.maxHealth) : 0.f;
	}

	void PlayerStatusWidget::HandleReverbChanged(const ReverbChangedEvent& event)
	{
		_targetReverbRatio = event.maxReverb > 0.f ? event.currentReverb / event.maxReverb : 0.f;
	}

	void PlayerStatusWidget::HandleRhythmActionStarted(const RhythmJudgeResult& result)
	{
		if (result.judgeGrade != RhythmJudgeGrade::OffBeat)
			PlayBeatFeedback();
	}

	void PlayerStatusWidget::UpdateStatus(float deltaTime)
	{
		const float smoothingRatio = Math::CalcExponentialSmoothingRatio(StatusSmoothingSpeed, deltaTime);
		_healthRatio += (_targetHealthRatio - _healthRatio) * smoothingRatio;
		_reverbRatio += (_targetReverbRatio - _reverbRatio) * smoothingRatio;

		_healthImage->SetFillRatio(_healthRatio);
		_reverbImage->SetFillRatio(_reverbRatio);

		const bool isReverbFull = _reverbRatio >= FullReverbThreshold;
		_backgroundImage->SetVisible(isReverbFull == false);
		_fullBackgroundImage->SetVisible(isReverbFull);
		_fullReverbImage->SetVisible(isReverbFull);
		_thunderImage->SetVisible(isReverbFull);
	}

	void PlayerStatusWidget::UpdateShadow()
	{
		const float pulse = _beatSystem.HasPlaybackTime() ? BeatMath::EvaluateBeatIntervalPulse(_beatSystem.GetCurrentBeat(), 1.f) : 0.f;
		_shadowImage->SetPosition(BackgroundCenter + ShadowMoveOffset * pulse);
	}

	void PlayerStatusWidget::UpdateThunder()
	{
		if (_thunderImage->IsVisible() == false || _beatSystem.HasPlaybackTime() == false)
			return;

		const uint32 frameIndex = std::min(static_cast<uint32>(_beatSystem.GetBeatProgress() * ThunderTextureKeys.size()), static_cast<uint32>(ThunderTextureKeys.size() - 1));
		if (_thunderFrameIndex == frameIndex)
			return;

		_thunderFrameIndex = frameIndex;
		_thunderImage->SetTexture(ThunderTextureKeys[frameIndex]);
	}

	void PlayerStatusWidget::UpdateAfterImages()
	{
		if (_isAfterImageActive == false)
			return;

		const float elapsedBeats = _beatSystem.GetCurrentBeat() - _afterImageStartBeat;
		if (elapsedBeats < 0.f || elapsedBeats >= AfterImageDurationBeats)
		{
			_isAfterImageActive = false;
			for (AfterImageEntry& entry : _afterImages)
				entry.image->SetVisible(false);
			return;
		}

		const float moveRatio = std::sin(Math::GM_PI * elapsedBeats / AfterImageDurationBeats);
		for (AfterImageEntry& entry : _afterImages)
			entry.image->SetPosition(AfterImageCenter + entry.targetOffset * moveRatio);
	}

	void PlayerStatusWidget::SetImageGeometry(Image& image, const Vector2& center, const Vector2& size) const
	{
		image.SetPosition(center);
		image.SetSize(size);
	}
}
