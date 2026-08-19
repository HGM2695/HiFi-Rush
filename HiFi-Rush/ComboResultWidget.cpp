#include "ComboResultWidget.h"

#include "CanvasPanel.h"
#include "ChiStateMachineComponent.h"
#include "Image.h"
#include "MathUtil.h"

#include <array>
#include <cmath>
#include <optional>

namespace gm
{
	namespace
	{
		enum class ComboType
		{
			Humburker,
			GrandSlam,
			BreakDown,
			Tremolo,
			DashAttack,
			AirLaunch,
			RiseUp,
			HammerOn,
			AirCountDown,

			Count
		};

		struct ComboVisualInfo
		{
			const wchar_t*	textureKey = nullptr;
			Vector2			targetCenter{};
			Vector2			size{};
		};

		constexpr std::array<ComboVisualInfo, static_cast<uint32>(ComboType::Count)> ComboVisualInfos =
		{{
			{ L"Humburker", { 1442.5f, 514.375f }, { 250.f, 146.25f } },
			{ L"GrandSlam", { 1442.5f, 515.f }, { 250.f, 145.f } },
			{ L"BreakDown", { 1442.5f, 515.f }, { 250.f, 145.f } },
			{ L"Tremolo", { 1442.5f, 515.f }, { 250.f, 145.f } },
			{ L"DashAttack", { 1442.5f, 515.f }, { 250.f, 145.f } },
			{ L"AirLaunch", { 1442.5f, 514.375f }, { 250.f, 146.25f } },
			{ L"RiseUp", { 1442.5f, 515.f }, { 250.f, 145.f } },
			{ L"HammerOn", { 1442.5f, 515.f }, { 250.f, 145.f } },
			{ L"AirCountDown", { 1408.75f, 515.625f }, { 375.f, 146.25f } },
		}};

		constexpr float SlideDuration = 0.4f;
		constexpr float HoldDuration = 1.f;
		constexpr float TotalDuration = SlideDuration * 2.f + HoldDuration;

		std::optional<ComboType> FindComboType(ChiStateId stateId)
		{
			switch (stateId)
			{
			case ChiStateId::AttackWeak3:
				return ComboType::Humburker;
			case ChiStateId::AttackStrong2:
				return ComboType::GrandSlam;
			case ChiStateId::AttackWeakToStrong2:
				return ComboType::BreakDown;
			case ChiStateId::AttackStrongToWeak2:
				return ComboType::Tremolo;
			case ChiStateId::AttackWeakDash:
				return ComboType::DashAttack;
			case ChiStateId::AttackStrongDash:
				return ComboType::AirLaunch;
			case ChiStateId::AttackDelayedWeak2:
				return ComboType::RiseUp;
			case ChiStateId::AttackStump0:
				return ComboType::HammerOn;
			case ChiStateId::AttackSky3:
				return ComboType::AirCountDown;
			default:
				return std::nullopt;
			}
		}
	}

	ComboResultWidget::ComboResultWidget(ChiStateMachineComponent& stateMachine)
		: _stateMachine(stateMachine)
	{
	}

	std::unique_ptr<Widget> ComboResultWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);
		_comboImage = root->AddNamedChild<Image>(L"ComboName", L"Humburker");
		_comboImage->SetVisible(false);
		return root;
	}

	void ComboResultWidget::OnInitialize()
	{
		_stateMachine.OnRhythmActionStarted.Subscribe(_rhythmActionConnection,
			[this](const RhythmJudgeResult& result)
			{
				HandleRhythmActionStarted(result);
			});
	}

	void ComboResultWidget::OnTick(float deltaTime)
	{
		if (_isPlaying == false)
			return;

		_elapsedTime += deltaTime;
		if (_elapsedTime >= TotalDuration)
		{
			Stop();
			return;
		}

		float ratio = 1.f;
		if (_elapsedTime < SlideDuration)
			ratio = std::sin((_elapsedTime / SlideDuration) * Math::GM_PI * 0.5f);
		else if (_elapsedTime > SlideDuration + HoldDuration)
			ratio = std::sin(((TotalDuration - _elapsedTime) / SlideDuration) * Math::GM_PI * 0.5f);

		ApplyAnimation(ratio);
	}

	void ComboResultWidget::HandleRhythmActionStarted(const RhythmJudgeResult&)
	{
		const std::optional<ComboType> comboType = FindComboType(_stateMachine.GetCurrentStateId());
		if (comboType)
			Play(static_cast<uint32>(comboType.value()));
	}

	void ComboResultWidget::Play(uint32 comboIndex)
	{
		GM_ASSERT_RETURN(comboIndex < ComboVisualInfos.size(), "지원하지 않는 Combo Type입니다.");

		_comboIndex = comboIndex;
		_elapsedTime = 0.f;
		_isPlaying = true;
		_comboImage->SetTexture(ComboVisualInfos[_comboIndex].textureKey);
		_comboImage->SetVisible(true);
		ApplyAnimation(0.f);
	}

	void ComboResultWidget::Stop()
	{
		_elapsedTime = 0.f;
		_isPlaying = false;
		_comboImage->SetVisible(false);
	}

	void ComboResultWidget::ApplyAnimation(float ratio)
	{
		const ComboVisualInfo& info = ComboVisualInfos[_comboIndex];
		const Vector2 startCenter = info.targetCenter + Vector2{ info.size.x, 0.f };
		_comboImage->SetGeometry(startCenter + (info.targetCenter - startCenter) * ratio, info.size);
	}
}
