#include "QamilHealthWidget.h"

#include "BeatMath.h"
#include "BeatSystem.h"
#include "CanvasPanel.h"
#include "CombatTypes.h"
#include "HealthComponent.h"
#include "Image.h"
#include "MathUtil.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace gm
{
	namespace
	{
		constexpr std::array<const wchar_t*, 4> BackgroundTextureKeys{ L"Qamil_BG0", L"Qamil_BG1", L"Qamil_BG2", L"Qamil_BG3" };
		constexpr std::array<const wchar_t*, 4> BrokenBackgroundTextureKeys{ L"Qamil_BrokenBG0", L"Qamil_BrokenBG1", L"Qamil_BrokenBG2", L"Qamil_BrokenBG3" };
		constexpr std::array<const wchar_t*, 4> HealthTextureKeys{ L"Qamil_HP0", L"Qamil_HP1", L"Qamil_HP2", L"Qamil_HP3" };
		constexpr std::array<const wchar_t*, 4> DamageTextureKeys{ L"Qamil_DamageHP0", L"Qamil_DamageHP1", L"Qamil_DamageHP2", L"Qamil_DamageHP3" };
		constexpr std::array<const wchar_t*, 2> GoldTextureKeys{ L"Qamil_Gold0", L"Qamil_Gold1" };

		constexpr std::array<Vector2, 2> GoldCenters{ Vector2{ 390.f, 828.75f }, Vector2{ 1210.f, 828.75f } };
		constexpr Vector2 GoldSize{ 117.5f, 90.f };

		constexpr std::array<Vector2, 4> BackgroundCenters{ Vector2{ 497.5f, 828.125f }, Vector2{ 698.75f, 830.625f }, Vector2{ 900.f, 830.625f }, Vector2{ 1101.25f, 828.125f } };
		constexpr std::array<Vector2, 4> BackgroundSizes{ Vector2{ 197.5f, 46.25f }, Vector2{ 195.f, 41.25f }, Vector2{ 195.f, 41.25f }, Vector2{ 197.5f, 46.25f } };
		constexpr std::array<Vector2, 4> BrokenBackgroundCenters{ Vector2{ 497.5f, 828.75f }, Vector2{ 698.75f, 830.f }, Vector2{ 900.f, 830.f }, Vector2{ 1101.25f, 828.75f } };
		constexpr std::array<Vector2, 4> BrokenBackgroundSizes{ Vector2{ 197.5f, 52.5f }, Vector2{ 195.f, 45.f }, Vector2{ 195.f, 45.f }, Vector2{ 197.5f, 52.5f } };

		constexpr std::array<Vector2, 4> HealthCenters{ Vector2{ 498.75f, 828.125f }, Vector2{ 698.75f, 830.625f }, Vector2{ 900.f, 830.f }, Vector2{ 1100.f, 828.125f } };
		constexpr std::array<Vector2, 4> HealthSizes{ Vector2{ 192.5f, 43.75f }, Vector2{ 192.5f, 41.25f }, Vector2{ 192.5f, 40.f }, Vector2{ 192.5f, 43.75f } };
		constexpr std::array<Vector2, 4> DamageCenters{ Vector2{ 496.875f, 828.75f }, Vector2{ 700.f, 830.f }, Vector2{ 900.f, 830.f }, Vector2{ 1102.5f, 830.f } };
		constexpr std::array<Vector2, 4> DamageSizes{ Vector2{ 206.25f, 72.5f }, Vector2{ 200.f, 70.f }, Vector2{ 200.f, 70.f }, Vector2{ 205.f, 72.5f } };

		constexpr Vector2 NameCenter{ 804.375f, 858.75f };
		constexpr Vector2 NameSize{ 191.25f, 47.5f };
		constexpr Vector2 ShadowCenter{ 800.625f, 830.f };
		constexpr Vector2 ShadowSize{ 801.25f, 47.5f };
		constexpr Vector2 ShadowBeatOffset{ 0.f, 20.f };

		constexpr float DamageOverlayFadeSpeed = 4.f;
		constexpr float DamageEffectDuration = 0.25f;
		constexpr float DamageEffectCenterY = 823.75f;
		constexpr Vector2 DamageEffectSize{ 3.75f, 137.5f };
	}

	QamilHealthWidget::QamilHealthWidget(const BeatSystem& beatSystem, HealthComponent& healthComponent)
		: _beatSystem(beatSystem), _healthComponent(healthComponent), _healthRatio(healthComponent.GetHealthRatio())
	{
	}

	std::unique_ptr<Widget> QamilHealthWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);

		_shadowImage = root->AddNamedChild<Image>(L"Shadow", L"Qamil_Shadow");
		_shadowImage->SetGeometry(ShadowCenter, ShadowSize);

		for (uint32 goldIndex = 0; goldIndex < _goldImages.size(); ++goldIndex)
		{
			_goldImages[goldIndex] = root->AddNamedChild<Image>(L"Gold" + std::to_wstring(goldIndex), GoldTextureKeys[goldIndex]);
			_goldImages[goldIndex]->SetGeometry(GoldCenters[goldIndex], GoldSize);
		}

		for (uint32 segmentIndex = 0; segmentIndex < HealthSegmentCount; ++segmentIndex)
		{
			HealthSegmentEntry& segment = _healthSegments[segmentIndex];
			segment.background = root->AddNamedChild<Image>(L"Background" + std::to_wstring(segmentIndex), BackgroundTextureKeys[segmentIndex]);
			segment.background->SetGeometry(BackgroundCenters[segmentIndex], BackgroundSizes[segmentIndex]);
			segment.brokenBackground = root->AddNamedChild<Image>(L"BrokenBackground" + std::to_wstring(segmentIndex), BrokenBackgroundTextureKeys[segmentIndex]);
			segment.brokenBackground->SetGeometry(BrokenBackgroundCenters[segmentIndex], BrokenBackgroundSizes[segmentIndex]);
		}

		for (uint32 segmentIndex = 0; segmentIndex < HealthSegmentCount; ++segmentIndex)
		{
			HealthSegmentEntry& segment = _healthSegments[segmentIndex];
			segment.health = root->AddNamedChild<Image>(L"Health" + std::to_wstring(segmentIndex), HealthTextureKeys[segmentIndex]);
			segment.health->SetGeometry(HealthCenters[segmentIndex], HealthSizes[segmentIndex]);
			segment.health->SetFillMode(ImageFillMode::HorizontalReverse);
		}

		for (uint32 segmentIndex = 0; segmentIndex < HealthSegmentCount; ++segmentIndex)
		{
			HealthSegmentEntry& segment = _healthSegments[segmentIndex];
			segment.damage = root->AddNamedChild<Image>(L"Damage" + std::to_wstring(segmentIndex), DamageTextureKeys[segmentIndex]);
			segment.damage->SetGeometry(DamageCenters[segmentIndex], DamageSizes[segmentIndex]);
			segment.damage->SetFillMode(ImageFillMode::HorizontalReverse);
			segment.damage->SetVisible(false);
		}

		_nameImage = root->AddNamedChild<Image>(L"Name", L"Qamil_Name");
		_nameImage->SetGeometry(NameCenter, NameSize);

		for (uint32 effectIndex = 0; effectIndex < DamageEffectCount; ++effectIndex)
		{
			DamageEffectEntry& effect = _damageEffects[effectIndex];
			effect.image = root->AddNamedChild<Image>(L"DamageEffect" + std::to_wstring(effectIndex), L"Qamil_DamageEffect");
			effect.image->SetVisible(false);
		}

		UpdateHealthSegments();
		return root;
	}

	void QamilHealthWidget::OnInitialize()
	{
		_healthComponent.OnHealthChanged.Subscribe(_healthChangedConnection,
			[this](const HealthChangedEvent& event)
			{
				HandleHealthChanged(event);
			});
	}

	void QamilHealthWidget::OnTick(float deltaTime)
	{
		UpdateShadow();
		UpdateDamageOverlay(deltaTime);
		UpdateDamageEffects(deltaTime);
	}

	void QamilHealthWidget::HandleHealthChanged(const HealthChangedEvent& event)
	{
		const bool wasDamaged = event.currentHealth < event.previousHealth;
		_healthRatio = event.maxHealth > 0 ? static_cast<float>(event.currentHealth) / static_cast<float>(event.maxHealth) : 0.f;
		UpdateHealthSegments();

		if (event.currentHealth <= 0)
		{
			SetVisible(false);
			return;
		}

		if (wasDamaged)
			PlayDamageFeedback();
		else
			StopDamageOverlay();
	}

	void QamilHealthWidget::UpdateHealthSegments()
	{
		for (uint32 segmentIndex = 0; segmentIndex < HealthSegmentCount; ++segmentIndex)
		{
			const float fillRatio = CalculateSegmentFillRatio(segmentIndex);
			HealthSegmentEntry& segment = _healthSegments[segmentIndex];
			segment.background->SetVisible(fillRatio > 0.f);
			segment.brokenBackground->SetVisible(fillRatio <= 0.f);
			segment.health->SetFillRatio(fillRatio);
			segment.health->SetVisible(fillRatio > 0.f);
		}
	}

	void QamilHealthWidget::UpdateShadow()
	{
		const float pulse = _beatSystem.HasPlaybackTime() ? BeatMath::EvaluateBeatIntervalPulse(_beatSystem.GetCurrentBeat(), 1.f) : 0.f;
		_shadowImage->SetPosition(ShadowCenter + ShadowBeatOffset * pulse);
	}

	void QamilHealthWidget::PlayDamageFeedback()
	{
		_damageOverlayAlpha = 1.f;
		const float boundaryX = CalculateHealthBoundaryX();
		for (uint32 segmentIndex = 0; segmentIndex < HealthSegmentCount; ++segmentIndex)
		{
			const float rightX = DamageCenters[segmentIndex].x + DamageSizes[segmentIndex].x * 0.5f;
			const float fillRatio = std::clamp((rightX - boundaryX) / DamageSizes[segmentIndex].x, 0.f, 1.f);
			Image* damageImage = _healthSegments[segmentIndex].damage;
			damageImage->SetFillRatio(fillRatio);
			damageImage->SetOpacity(1.f);
			damageImage->SetVisible(fillRatio > 0.f);
		}

		DamageEffectEntry& effect = _damageEffects[_nextDamageEffectIndex];
		effect.elapsed = 0.f;
		effect.isActive = true;
		effect.image->SetGeometry(Vector2{ boundaryX, DamageEffectCenterY }, Vector2{ DamageEffectSize.x, DamageEffectSize.y * 0.5f });
		effect.image->SetOpacity(1.f);
		effect.image->SetVisible(true);
		_nextDamageEffectIndex = (_nextDamageEffectIndex + 1) % DamageEffectCount;
	}

	void QamilHealthWidget::UpdateDamageOverlay(float deltaTime)
	{
		if (_damageOverlayAlpha <= 0.f)
			return;

		_damageOverlayAlpha = (std::max)(_damageOverlayAlpha - deltaTime * DamageOverlayFadeSpeed, 0.f);
		for (HealthSegmentEntry& segment : _healthSegments)
		{
			segment.damage->SetOpacity(_damageOverlayAlpha);
			if (_damageOverlayAlpha <= 0.f)
				segment.damage->SetVisible(false);
		}
	}

	void QamilHealthWidget::UpdateDamageEffects(float deltaTime)
	{
		for (DamageEffectEntry& effect : _damageEffects)
		{
			if (effect.isActive == false)
				continue;

			effect.elapsed += deltaTime;
			if (effect.elapsed >= DamageEffectDuration)
			{
				effect.isActive = false;
				effect.image->SetVisible(false);
				continue;
			}

			const float ratio = std::sin(2.f * Math::GM_PI * effect.elapsed);
			effect.image->SetSize(Vector2{ DamageEffectSize.x * (1.f - ratio), DamageEffectSize.y * (0.5f + 0.5f * ratio) });
			effect.image->SetOpacity(1.f - ratio);
		}
	}

	void QamilHealthWidget::StopDamageOverlay()
	{
		_damageOverlayAlpha = 0.f;
		for (HealthSegmentEntry& segment : _healthSegments)
			segment.damage->SetVisible(false);
	}

	float QamilHealthWidget::CalculateSegmentFillRatio(uint32 segmentIndex) const
	{
		const float segmentSize = 1.f / static_cast<float>(HealthSegmentCount);
		const float segmentStartRatio = static_cast<float>(HealthSegmentCount - segmentIndex - 1) * segmentSize;
		return std::clamp((_healthRatio - segmentStartRatio) / segmentSize, 0.f, 1.f);
	}

	float QamilHealthWidget::CalculateHealthBoundaryX() const
	{
		for (uint32 segmentIndex = 0; segmentIndex < HealthSegmentCount; ++segmentIndex)
		{
			const float fillRatio = CalculateSegmentFillRatio(segmentIndex);
			if (fillRatio <= 0.f)
				continue;

			return HealthCenters[segmentIndex].x + HealthSizes[segmentIndex].x * (0.5f - fillRatio);
		}

		return HealthCenters.back().x + HealthSizes.back().x * 0.5f;
	}
}
