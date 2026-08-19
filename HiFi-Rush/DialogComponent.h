#pragma once

#include "Component.h"
#include "DialogTypes.h"

#include <vector>

namespace gm
{
	class AudioComponent;
	class PlayerControlComponent;

	class DialogComponent final : public Component
	{
	public:
		DialogComponent(AudioComponent& audioComponent, PlayerControlComponent& playerControlComponent);

		bool						SetSequences(std::vector<DialogSequenceData> sequences);
		bool						PlaySequence(const std::wstring& sequenceId);
		bool						SelectBranch(const std::wstring& branchKey);
		void						Stop();

		bool						IsPlaying() const { return _playbackState != DialogPlaybackState::Idle; }
		bool						IsWaitingForBranch() const { return _playbackState == DialogPlaybackState::WaitingForBranch; }
		bool						IsVoicePlaying() const;
		float						GetVoiceSpectrumAmplitude() const;
		const DialogSequenceData*	GetCurrentSequence() const { return _currentSequence; }
		const DialogLineData*		GetCurrentLine() const { return _currentLine; }

		EventPublisher<DialogComponent, DialogLineChangedEvent>		OnLineChanged;
		EventPublisher<DialogComponent, DialogBranchRequestedEvent>	OnBranchRequested;
		EventPublisher<DialogComponent, DialogFinishedEvent>		OnFinished;

	protected:
		void OnTick(float deltaTime) override;

	private:
		const DialogSequenceData*	FindSequence(const std::wstring& sequenceId) const;
		const DialogLineData*		FindLine(const DialogSequenceData& sequence, const std::wstring& lineId) const;
		void						BeginLine(const DialogLineData& line);
		void						AdvanceLine();
		void						Finish(bool completed);
		void						BlockPlayerControls();
		void						ApplyControlsAfterReleasePoint();
		void						ApplyPlayerControls(PlayerControlMask controls);
		float						CalculateFallbackDuration(const DialogLineData& line) const;

		AudioComponent&					_audioComponent;
		PlayerControlComponent&			_playerControlComponent;
		std::vector<DialogSequenceData>	_sequences{};
		const DialogSequenceData*		_currentSequence = nullptr;
		const DialogLineData*			_currentLine = nullptr;
		DialogPlaybackState				_playbackState = DialogPlaybackState::Idle;
		float							_lineElapsed = 0.f;
		float							_lineDuration = 0.f;
	};
}
