#pragma once

#include "DialogTypes.h"

#include <vector>

namespace gm
{
	namespace HiFiRushDialogSequenceIds
	{
		inline constexpr wchar_t TutorialIntro[] = L"TutorialIntro";
		inline constexpr wchar_t TutorialWeakRhythm[] = L"TutorialWeakRhythm";
		inline constexpr wchar_t StrongAttackIntro[] = L"StrongAttackIntro";
		inline constexpr wchar_t StrongAttackRhythm[] = L"StrongAttackRhythm";
		inline constexpr wchar_t SaverEncounter[] = L"SaverEncounter";
		inline constexpr wchar_t ShuffleTutorial[] = L"ShuffleTutorial";
	}

	namespace HiFiRushDialogBranchKeys
	{
		inline constexpr wchar_t Retry[] = L"Retry";
		inline constexpr wchar_t Perfect[] = L"Perfect";
	}

	std::vector<DialogSequenceData> CreateHiFiRushDialogSequences();
}
