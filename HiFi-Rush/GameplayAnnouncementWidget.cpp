#include "GameplayAnnouncementWidget.h"

#include "BeatSystem.h"
#include "CanvasPanel.h"
#include "Image.h"
#include "MathUtil.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace gm
{
	namespace
	{
		enum class AnnouncementAnimation
		{
			MoveAndGrow,
			GrowAndShrink,
		};

		struct AnnouncementVisualInfo
		{
			const wchar_t*			textureKey = nullptr;
			Vector2					startCenter{};
			Vector2					targetCenter{};
			Vector2					startSize{};
			Vector2					targetSize{};
			float					opacity = 1.f;
			float					durationBeats = 1.f;
			AnnouncementAnimation	animation = AnnouncementAnimation::MoveAndGrow;
		};

		constexpr std::array<AnnouncementVisualInfo, static_cast<uint32>(GameplayAnnouncementType::Count)> AnnouncementVisualInfos =
		{{
			{ L"T_VFX_tk_word_FIGHT", { 800.f, 648.75f }, { 800.f, 412.5f }, { 375.f, 162.5f }, { 750.f, 325.f }, 0.5f, 2.5f, AnnouncementAnimation::MoveAndGrow },
			{ L"Font_Yeah", { 825.f, 450.f }, { 825.f, 450.f }, { 1000.f, 600.f }, { 1250.f, 750.f }, 0.3f, 1.5f, AnnouncementAnimation::GrowAndShrink },
		}};

		const AnnouncementVisualInfo& GetAnnouncementVisualInfo(GameplayAnnouncementType type)
		{
			return AnnouncementVisualInfos[static_cast<uint32>(type)];
		}
	}

	GameplayAnnouncementWidget::GameplayAnnouncementWidget(const BeatSystem& beatSystem)
		: _beatSystem(beatSystem)
	{
	}

	void GameplayAnnouncementWidget::Play(GameplayAnnouncementType type)
	{
		GM_ASSERT_RETURN(type < GameplayAnnouncementType::Count, "지원하지 않는 Gameplay Announcement Type입니다.");

		const AnnouncementVisualInfo& info = GetAnnouncementVisualInfo(type);
		_currentType = type;
		_startBeat = _beatSystem.HasPlaybackTime() ? std::optional<float>{ _beatSystem.GetCurrentBeat() } : std::nullopt;
		_isPlaying = true;
		_announcementImage->SetTexture(info.textureKey);
		_announcementImage->SetOpacity(info.opacity);
		_announcementImage->SetVisible(true);
		ApplyAnimation(0.f);
	}

	void GameplayAnnouncementWidget::Stop()
	{
		_currentType = GameplayAnnouncementType::Count;
		_isPlaying = false;
		_startBeat.reset();
		_announcementImage->SetVisible(false);
	}

	std::unique_ptr<Widget> GameplayAnnouncementWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);
		_announcementImage = root->AddNamedChild<Image>(L"Announcement", L"T_VFX_tk_word_FIGHT");
		_announcementImage->SetVisible(false);
		return root;
	}

	void GameplayAnnouncementWidget::OnTick(float)
	{
		if (_isPlaying == false || _beatSystem.HasPlaybackTime() == false)
			return;

		if (_startBeat.has_value() == false)
		{
			_startBeat = _beatSystem.GetCurrentBeat();
		}

		const AnnouncementVisualInfo& info = GetAnnouncementVisualInfo(_currentType);
		const float elapsedBeats = _beatSystem.GetCurrentBeat() - _startBeat.value();
		if (elapsedBeats < 0.f || elapsedBeats >= info.durationBeats)
		{
			Stop();
			return;
		}

		ApplyAnimation(std::clamp(elapsedBeats / info.durationBeats, 0.f, 1.f));
	}

	void GameplayAnnouncementWidget::ApplyAnimation(float ratio)
	{
		const AnnouncementVisualInfo& info = GetAnnouncementVisualInfo(_currentType);
		float animationRatio = ratio;
		switch (info.animation)
		{
		case AnnouncementAnimation::MoveAndGrow:
			animationRatio = 1.f - std::pow(1.f - ratio, 3.f);
			break;

		case AnnouncementAnimation::GrowAndShrink:
			animationRatio = std::sin(Math::GM_PI * ratio);
			break;
		}

		_announcementImage->SetGeometry(
			info.startCenter + (info.targetCenter - info.startCenter) * animationRatio,
			info.startSize + (info.targetSize - info.startSize) * animationRatio);
	}
}
