#include "HibikiWidget.h"

#include "BeatSystem.h"
#include "CanvasPanel.h"
#include "ChiStateMachineComponent.h"
#include "Image.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace gm
{
	namespace
	{
		struct ImageLayout
		{
			Vector2 center{};
			Vector2 size{};
		};

		constexpr ImageLayout BackgroundLayout{ { 1037.5f, 698.125f }, { 1125.f, 496.25f } };
		constexpr ImageLayout FontLayout{ { 1157.5f, 596.25f }, { 430.f, 175.f } };
		constexpr ImageLayout OriginalLayout{ { 1321.875f, 553.75f }, { 493.75f, 687.5f } };
		constexpr std::array<ImageLayout, 3> AfterImageLayouts =
		{{
			{ { 1325.625f, 550.f }, { 493.75f, 687.5f } },
			{ { 1329.375f, 546.25f }, { 493.75f, 687.5f } },
			{ { 1333.125f, 542.5f }, { 493.75f, 687.5f } },
		}};
		constexpr std::array<Color, 3> AfterImageColors =
		{{
			Colors::White,
			ColorFromRGBA(253, 153, 2),
			ColorFromRGBA(251, 200, 4),
		}};
		constexpr float OffscreenCenterX = 2400.f;
		constexpr float PresentationDurationBeats = 2.5f;
		constexpr float MovementInterpolationSpeed = 21.4005f;
		constexpr float BackgroundActivationDistance = 93.75f;
		constexpr float AfterImageActivationDistance = 62.5f;
	}

	HibikiWidget::HibikiWidget(const BeatSystem& beatSystem, ChiStateMachineComponent& stateMachine)
		: _beatSystem(beatSystem), _stateMachine(stateMachine)
	{
	}

	void HibikiWidget::Play()
	{
		_startBeat = _beatSystem.HasPlaybackTime() ? std::optional<float>{ _beatSystem.GetCurrentBeat() } : std::nullopt;
		_elapsedTime = 0.f;
		_isPlaying = true;
		ResetPresentation();
	}

	void HibikiWidget::Stop()
	{
		_isPlaying = false;
		_startBeat.reset();
		_backgroundImage->SetVisible(false);
		_originalImage->SetVisible(false);
		_fontImage->SetVisible(false);
		for (Image* image : _afterImages)
			image->SetVisible(false);
	}

	std::unique_ptr<Widget> HibikiWidget::BuildWidgetTree()
	{
		auto root = CreateNamedRootWidget<CanvasPanel>(RootWidgetName);
		_backgroundImage = root->AddNamedChild<Image>(L"Background", L"BackGround");
		for (size_t reverseIndex = _afterImages.size(); reverseIndex > 0; --reverseIndex)
		{
			const size_t index = reverseIndex - 1;
			_afterImages[index] = root->AddNamedChild<Image>(L"AfterImage" + std::to_wstring(index), L"Chi");
			_afterImages[index]->SetColorBlend(AfterImageColors[index], 1.f);
		}
		_originalImage = root->AddNamedChild<Image>(L"Original", L"Chi");
		_fontImage = root->AddNamedChild<Image>(L"Font", L"Font_Hibiki");
		ResetPresentation();
		Stop();
		return root;
	}

	void HibikiWidget::OnInitialize()
	{
		_stateMachine.OnStateChanged.Subscribe(_stateChangedConnection, [this](const ChiStateChangedEvent& event) { HandleStateChanged(event); });
		if (_stateMachine.GetCurrentStateId() == ChiStateId::HibikiReady)
			Play();
	}

	void HibikiWidget::OnTick(float deltaTime)
	{
		if (_isPlaying == false)
			return;

		_elapsedTime += std::max(0.f, deltaTime);
		const float elapsedBeats = _startBeat && _beatSystem.HasPlaybackTime() ? _beatSystem.GetCurrentBeat() - _startBeat.value() : _elapsedTime / std::max(_beatSystem.GetSecondsPerBeat(), 0.0001f);
		if (elapsedBeats < 0.f || elapsedBeats >= PresentationDurationBeats)
		{
			Stop();
			return;
		}

		const float interpolationRatio = 1.f - std::exp(-MovementInterpolationSpeed * std::max(0.f, deltaTime));
		MoveImage(*_backgroundImage, BackgroundLayout.center, interpolationRatio);
		MoveImage(*_originalImage, OriginalLayout.center, interpolationRatio);
		if (_fontImage->IsVisible())
			MoveImage(*_fontImage, FontLayout.center, interpolationRatio);
		for (size_t index = 0; index < _afterImages.size(); ++index)
		{
			if (_afterImages[index]->IsVisible())
				MoveImage(*_afterImages[index], AfterImageLayouts[index].center, interpolationRatio);
		}

		if (_afterImages[2]->IsVisible())
			return;
		if (_afterImages[1]->IsVisible() && AfterImageLayouts[1].center.x - _afterImages[1]->GetPosition().x > -AfterImageActivationDistance)
			ActivateImage(*_afterImages[2]);
		else if (_afterImages[0]->IsVisible() && AfterImageLayouts[0].center.x - _afterImages[0]->GetPosition().x > -AfterImageActivationDistance)
			ActivateImage(*_afterImages[1]);
		else if (BackgroundLayout.center.x - _backgroundImage->GetPosition().x > -BackgroundActivationDistance)
		{
			ActivateImage(*_afterImages[0]);
			ActivateImage(*_fontImage);
		}
	}

	void HibikiWidget::HandleStateChanged(const ChiStateChangedEvent& event)
	{
		if (event.currentStateId == ChiStateId::HibikiReady)
			Play();
	}

	void HibikiWidget::ResetPresentation()
	{
		_backgroundImage->SetGeometry({ OffscreenCenterX, BackgroundLayout.center.y }, BackgroundLayout.size);
		_originalImage->SetGeometry({ OffscreenCenterX, OriginalLayout.center.y }, OriginalLayout.size);
		_fontImage->SetGeometry({ OffscreenCenterX, FontLayout.center.y }, FontLayout.size);
		for (size_t index = 0; index < _afterImages.size(); ++index)
			_afterImages[index]->SetGeometry({ OffscreenCenterX, AfterImageLayouts[index].center.y }, AfterImageLayouts[index].size);
		_backgroundImage->SetVisible(true);
		_originalImage->SetVisible(true);
		_fontImage->SetVisible(false);
		for (Image* image : _afterImages)
			image->SetVisible(false);
	}

	void HibikiWidget::MoveImage(Image& image, const Vector2& targetCenter, float interpolationRatio)
	{
		image.SetPosition(Vector2::Lerp(image.GetPosition(), targetCenter, interpolationRatio));
	}

	void HibikiWidget::ActivateImage(Image& image)
	{
		image.SetVisible(true);
	}
}
