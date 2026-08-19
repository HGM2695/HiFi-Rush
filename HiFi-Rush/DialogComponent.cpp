#include "DialogComponent.h"

#include "Application.h"
#include "AudioComponent.h"
#include "AudioStatics.h"
#include "PlayerControlComponent.h"
#include "Resources.h"
#include "SoundWave.h"

#include <algorithm>
#include <unordered_set>

namespace gm
{
	namespace
	{
		constexpr float VoiceEndDelay = 0.4f;
		constexpr float MinimumFallbackDuration = 1.5f;
		constexpr float MaximumFallbackDuration = 5.f;
		constexpr float SecondsPerCharacter = 0.055f;
	}

	DialogComponent::DialogComponent(AudioComponent& audioComponent, PlayerControlComponent& playerControlComponent)
		: _audioComponent(audioComponent), _playerControlComponent(playerControlComponent)
	{
		_audioComponent.SetSpectrumAnalysisEnabled(true);
	}

	bool DialogComponent::SetSequences(std::vector<DialogSequenceData> sequences)
	{
		GM_ASSERT_RETURN_VAL(IsPlaying() == false, false, "대화 재생 중에는 Sequence 데이터를 교체할 수 없습니다.");

		_sequences = std::move(sequences);
		return true;
	}

	bool DialogComponent::PlaySequence(const std::wstring& sequenceId)
	{
		const DialogSequenceData* sequence = FindSequence(sequenceId);
		GM_ASSERT_RETURN_VAL(sequence, false, "Dialog Sequence를 찾을 수 없습니다. id=%ls", sequenceId.c_str());

		const DialogLineData* entryLine = FindLine(*sequence, sequence->entryLineId);
		GM_ASSERT_RETURN_VAL(entryLine, false, "Dialog Sequence의 시작 대사를 찾을 수 없습니다. sequence=%ls", sequenceId.c_str());

		if (IsPlaying())
			Finish(false);

		_currentSequence = sequence;
		BlockPlayerControls();
		BeginLine(*entryLine);
		return true;
	}

	bool DialogComponent::SelectBranch(const std::wstring& branchKey)
	{
		GM_ASSERT_RETURN_VAL(_currentLine && _playbackState == DialogPlaybackState::WaitingForBranch, false, "선택을 기다리는 Dialog Branch가 없습니다.");

		const auto iter = std::find_if(_currentLine->branches.begin(), _currentLine->branches.end(),
			[&branchKey](const DialogBranchData& branch)
			{
				return branch.key == branchKey;
			});
		GM_ASSERT_RETURN_VAL(iter != _currentLine->branches.end(), false, "Dialog Branch를 찾을 수 없습니다. key=%ls", branchKey.c_str());

		if (iter->nextLineId.empty())
		{
			Finish(true);
			return true;
		}

		const DialogLineData* nextLine = FindLine(*_currentSequence, iter->nextLineId);
		GM_ASSERT_RETURN_VAL(nextLine, false, "Dialog Branch의 다음 대사를 찾을 수 없습니다. line=%ls", iter->nextLineId.c_str());
		BlockPlayerControls();
		BeginLine(*nextLine);
		return true;
	}

	void DialogComponent::Stop()
	{
		if (IsPlaying())
			Finish(false);
	}

	bool DialogComponent::IsVoicePlaying() const
	{
		return _playbackState == DialogPlaybackState::PlayingVoice && _audioComponent.IsPlaying();
	}

	float DialogComponent::GetVoiceSpectrumAmplitude() const
	{
		return IsVoicePlaying() ? _audioComponent.GetSpectrumAmplitude() : 0.f;
	}

	void DialogComponent::OnTick(float deltaTime)
	{
		switch (_playbackState)
		{
		case DialogPlaybackState::Idle:
		case DialogPlaybackState::WaitingForBranch:
			return;

		case DialogPlaybackState::PlayingVoice:
			if (_audioComponent.IsPlaying())
				return;

			_playbackState = DialogPlaybackState::WaitingForLineEnd;
			_lineElapsed = 0.f;
			_lineDuration = VoiceEndDelay;
			return;

		case DialogPlaybackState::WaitingForLineEnd:
			_lineElapsed += std::max(0.f, deltaTime);
			if (_lineElapsed >= _lineDuration)
				AdvanceLine();
			return;
		}
	}

	const DialogSequenceData* DialogComponent::FindSequence(const std::wstring& sequenceId) const
	{
		const auto iter = std::find_if(_sequences.begin(), _sequences.end(),
			[&sequenceId](const DialogSequenceData& sequence)
			{
				return sequence.id == sequenceId;
			});
		return iter != _sequences.end() ? &(*iter) : nullptr;
	}

	const DialogLineData* DialogComponent::FindLine(const DialogSequenceData& sequence, const std::wstring& lineId) const
	{
		const auto iter = std::find_if(sequence.lines.begin(), sequence.lines.end(),
			[&lineId](const DialogLineData& line)
			{
				return line.id == lineId;
			});
		return iter != sequence.lines.end() ? &(*iter) : nullptr;
	}

	void DialogComponent::BeginLine(const DialogLineData& line)
	{
		_audioComponent.Stop();
		_currentLine = &line;
		_playbackState = DialogPlaybackState::WaitingForLineEnd;
		_lineElapsed = 0.f;
		_lineDuration = CalculateFallbackDuration(line);

		if (line.voiceResourceKey.empty() == false)
		{
			const std::shared_ptr<SoundWave> voice = APPLICATION.GetResources().Find<SoundWave>(line.voiceResourceKey);
			if (voice)
			{
				_audioComponent.SetSound(voice);
				_audioComponent.SetVolume(line.voiceVolume);
				_audioComponent.Play();
				_playbackState = DialogPlaybackState::PlayingVoice;
			}
		}

		if (line.soundEffectResourceKey.empty() == false)
			PlaySound2D(line.soundEffectResourceKey, line.soundEffectVolume);

		DialogLineChangedEvent event{};
		event.line = _currentLine;
		OnLineChanged.Publish(event);
	}

	void DialogComponent::AdvanceLine()
	{
		if (_currentLine->branches.empty() == false)
		{
			_playbackState = DialogPlaybackState::WaitingForBranch;
			if (_currentSequence->controlReleasePoint == DialogControlReleasePoint::BranchRequested)
				ApplyControlsAfterReleasePoint();

			DialogBranchRequestedEvent event{};
			event.sequence = _currentSequence;
			event.line = _currentLine;
			OnBranchRequested.Publish(event);
			return;
		}

		if (_currentLine->nextLineId.empty())
		{
			Finish(true);
			return;
		}

		const DialogLineData* nextLine = FindLine(*_currentSequence, _currentLine->nextLineId);
		GM_ASSERT_RETURN(nextLine, "Dialog의 다음 대사를 찾을 수 없습니다. line=%ls", _currentLine->nextLineId.c_str());
		BeginLine(*nextLine);
	}

	void DialogComponent::Finish(bool completed)
	{
		const std::wstring sequenceId = _currentSequence ? _currentSequence->id : std::wstring{};
		_audioComponent.Stop();
		_playerControlComponent.ReleaseControls(this);

		_currentSequence = nullptr;
		_currentLine = nullptr;
		_playbackState = DialogPlaybackState::Idle;
		_lineElapsed = 0.f;
		_lineDuration = 0.f;

		DialogFinishedEvent event{};
		event.sequenceId = sequenceId;
		event.completed = completed;
		OnFinished.Publish(event);
	}

	void DialogComponent::BlockPlayerControls()
	{
		if (_currentSequence == nullptr)
			return;

		ApplyPlayerControls(_currentSequence->blockedPlayerControls);
	}

	void DialogComponent::ApplyControlsAfterReleasePoint()
	{
		if (_currentSequence == nullptr)
			return;

		ApplyPlayerControls(_currentSequence->blockedPlayerControlsAfterRelease);
	}

	void DialogComponent::ApplyPlayerControls(PlayerControlMask controls)
	{
		if (controls == PlayerControl::None)
		{
			_playerControlComponent.ReleaseControls(this);
			return;
		}

		_playerControlComponent.BlockControls(this, controls);
	}

	float DialogComponent::CalculateFallbackDuration(const DialogLineData& line) const
	{
		return std::clamp(static_cast<float>(line.text.size()) * SecondsPerCharacter, MinimumFallbackDuration, MaximumFallbackDuration);
	}
}
