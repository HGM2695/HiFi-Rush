#include "HiFiRushDialogData.h"

namespace gm
{
	namespace
	{
		DialogLineData MakeLine(const wchar_t* id, const wchar_t* speakerName, const wchar_t* text, DialogPortrait portrait, const wchar_t* voiceResourceKey, float voiceVolume = 1.f)
		{
			DialogLineData line{};
			line.id = id;
			line.speakerName = speakerName;
			line.text = text;
			line.portrait = portrait;
			line.voiceResourceKey = voiceResourceKey;
			line.voiceVolume = voiceVolume;
			return line;
		}
	}

	std::vector<DialogSequenceData> CreateHiFiRushDialogSequences()
	{
		DialogSequenceData tutorialIntro{};
		tutorialIntro.id = HiFiRushDialogSequenceIds::TutorialIntro;
		tutorialIntro.entryLineId = L"Tuto1";
		tutorialIntro.blockedPlayerControls = PlayerControl::All;
		tutorialIntro.lines =
		{
			MakeLine(L"Tuto1", L"아나운서", L"자, 다들 신나게 놀아볼 준비 됐나요?", DialogPortrait::Announcer, L"DialogVoice.Tuto1"),
			MakeLine(L"Tuto2", L"차이", L"워우...", DialogPortrait::Chi, L"DialogVoice.Tuto2"),
			MakeLine(L"Tuto3", L"아나운서", L"여기 보시는 차이의 공격은 전부 음악과 싱크됩니다.\n다들 직접 보고 싶으시죠?", DialogPortrait::Announcer, L"DialogVoice.Tuto3"),
			MakeLine(L"Tuto4", L"아나운서", L"우선 약공격으로 시작하죠. 빠르게 콤보를 연결하는 단비트 공격입니다.", DialogPortrait::Announcer, L"DialogVoice.Tuto4"),
			MakeLine(L"Tuto5", L"아나운서", L"자, 한번 해보시죠.", DialogPortrait::Announcer, L"DialogVoice.Tuto5"),
		};
		for (size_t lineIndex = 0; lineIndex + 1 < tutorialIntro.lines.size(); ++lineIndex)
			tutorialIntro.lines[lineIndex].nextLineId = tutorialIntro.lines[lineIndex + 1].id;

		DialogSequenceData tutorialWeakRhythm{};
		tutorialWeakRhythm.id = HiFiRushDialogSequenceIds::TutorialWeakRhythm;
		tutorialWeakRhythm.entryLineId = L"Tuto6";
		tutorialWeakRhythm.blockedPlayerControls = PlayerControl::All;
		tutorialWeakRhythm.controlReleasePoint = DialogControlReleasePoint::BranchRequested;
		tutorialWeakRhythm.lines =
		{
			MakeLine(L"Tuto6", L"아나운서", L"그럼 이제 음악의 비트에 맞춰 콤보를 시도하세요.\n그게 완벽한 타이밍입니다.", DialogPortrait::Announcer, L"DialogVoice.Tuto6"),
			MakeLine(L"Tuto7Retry", L"아나운서", L"방금 건 없었던 일로 하고, 다시 한번 해보시죠.", DialogPortrait::Announcer, L"DialogVoice.Tuto7_OneMore"),
			MakeLine(L"Tuto7Perfect", L"아나운서", L"이야! 완벽한 타이밍이었어요!", DialogPortrait::Announcer, L"DialogVoice.Tuto7_Perfect"),
			MakeLine(L"Tuto8", L"차이", L"오 예!", DialogPortrait::Chi, L"DialogVoice.Tuto8"),
			MakeLine(L"Tuto9", L"아나운서", L"공격은 항상 비트에 맞춰지지만, 비트에 정확하게 맞춰 공격 버튼을 누르면\n보너스가 주어집니다.", DialogPortrait::Announcer, L"DialogVoice.Tuto9"),
		};
		tutorialWeakRhythm.lines[0].branches =
		{
			{ HiFiRushDialogBranchKeys::Retry, L"Tuto7Retry" },
			{ HiFiRushDialogBranchKeys::Perfect, L"Tuto7Perfect" },
		};
		tutorialWeakRhythm.lines[1].branches = tutorialWeakRhythm.lines[0].branches;
		tutorialWeakRhythm.lines[2].soundEffectResourceKey = L"DialogSFX.Applause";
		tutorialWeakRhythm.lines[2].soundEffectVolume = 0.2f;
		tutorialWeakRhythm.lines[2].nextLineId = L"Tuto8";
		tutorialWeakRhythm.lines[3].nextLineId = L"Tuto9";

		DialogSequenceData strongAttackIntro{};
		strongAttackIntro.id = HiFiRushDialogSequenceIds::StrongAttackIntro;
		strongAttackIntro.entryLineId = L"StrongTuto0";
		strongAttackIntro.blockedPlayerControls = PlayerControl::All;
		strongAttackIntro.lines =
		{
			MakeLine(L"StrongTuto0", L"아나운서", L"이번엔 엄청난 피해를 줄 수 있는 강공격을 알아보죠.", DialogPortrait::Announcer, L"DialogVoice.StrongTuto0", 0.5f),
			MakeLine(L"StrongTuto1", L"아나운서", L"이 공격에는 두 개의 비트가 필요하니, 중간에 있는 비트를 잘 들어 보세요.", DialogPortrait::Announcer, L"DialogVoice.StrongTuto1", 0.5f),
			MakeLine(L"StrongTuto2", L"차이", L"할 수 있어.", DialogPortrait::Chi, L"DialogVoice.StrongTuto2", 0.5f),
		};
		strongAttackIntro.lines[0].nextLineId = L"StrongTuto1";
		strongAttackIntro.lines[1].nextLineId = L"StrongTuto2";

		DialogSequenceData strongAttackRhythm{};
		strongAttackRhythm.id = HiFiRushDialogSequenceIds::StrongAttackRhythm;
		strongAttackRhythm.entryLineId = L"StrongTuto3";
		strongAttackRhythm.blockedPlayerControls = PlayerControl::All;
		strongAttackRhythm.controlReleasePoint = DialogControlReleasePoint::BranchRequested;
		strongAttackRhythm.lines =
		{
			MakeLine(L"StrongTuto3", L"아나운서", L"좋습니다. 그럼 리듬에 맞춰 강 콤보를 해 보죠.\n이런 타이밍을 노리면 됩니다!", DialogPortrait::Announcer, L"DialogVoice.StrongTuto3", 0.5f),
			MakeLine(L"StrongTutoRetry", L"아나운서", L"이게 맞아?", DialogPortrait::Announcer, L"DialogVoice.Tuto7_OneMore"),
			MakeLine(L"StrongTutoPerfect", L"아나운서", L"완전 프로군요! 대단해요!", DialogPortrait::Announcer, L"DialogVoice.StrongTuto4", 0.7f),
		};
		strongAttackRhythm.lines[0].branches =
		{
			{ HiFiRushDialogBranchKeys::Retry, L"StrongTutoRetry" },
			{ HiFiRushDialogBranchKeys::Perfect, L"StrongTutoPerfect" },
		};
		strongAttackRhythm.lines[1].branches = strongAttackRhythm.lines[0].branches;
		strongAttackRhythm.lines[2].soundEffectResourceKey = L"DialogSFX.Applause";
		strongAttackRhythm.lines[2].soundEffectVolume = 0.2f;

		DialogSequenceData saverEncounter{};
		saverEncounter.id = HiFiRushDialogSequenceIds::SaverEncounter;
		saverEncounter.entryLineId = L"Saver0";
		saverEncounter.blockedPlayerControls = PlayerControl::All;
		saverEncounter.lines =
		{
			MakeLine(L"Saver0", L"SBR-001", L"저놈 불량품이다!", DialogPortrait::Saver, L"DialogVoice.Saver0"),
			MakeLine(L"Saver1", L"SBR-001", L"녀석을 없애!", DialogPortrait::Saver, L"DialogVoice.Saver1"),
		};
		saverEncounter.lines[0].nextLineId = L"Saver1";

		DialogSequenceData shuffleTutorial{};
		shuffleTutorial.id = HiFiRushDialogSequenceIds::ShuffleTutorial;
		shuffleTutorial.entryLineId = L"Shuffle";
		shuffleTutorial.blockedPlayerControls = PlayerControl::All;
		shuffleTutorial.lines =
		{
			MakeLine(L"Shuffle", L"아나운서", L"강공격과 약공격을 연계해서 더 많은 콤보와 비트 히트를 성공시키는 건 어떨까요?\n자, 한번 해봅시다.", DialogPortrait::Announcer, L"DialogVoice.Shuffle"),
		};

		return
		{
			std::move(tutorialIntro),
			std::move(tutorialWeakRhythm),
			std::move(strongAttackIntro),
			std::move(strongAttackRhythm),
			std::move(saverEncounter),
			std::move(shuffleTutorial),
		};
	}
}
