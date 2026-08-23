#pragma once

#include <array>
#include <span>

namespace gm
{
	struct RhythmBGMDesc
	{
		const wchar_t*	commonResourceKey = nullptr;
		const wchar_t*	fileName = nullptr;
		float			bpm = 0.f;
		float			songOffsetSeconds = 0.f;
		float			volume = 1.f;
	};

	struct SoundResourceDesc
	{
		const wchar_t*	resourceKey = nullptr;
		const wchar_t*	fileName = nullptr;
	};

	namespace HiFiRushBGM
	{
		inline constexpr RhythmBGMDesc Tutorial{ L"Tutorial.BGM", L"metroSound.mp3", 136.f, 0.07f, 1.f };
		inline constexpr RhythmBGMDesc TutorialRoadUp{ L"Tutorial.RoadUp.BGM", L"TriggerStart.mp3", 136.f, 0.05f, 0.3f };
		inline constexpr RhythmBGMDesc Outside{ L"Outside.BGM", L"OpenMapSound2.mp3", 136.f, -0.06f, 1.f };
		inline constexpr RhythmBGMDesc Qamil{ L"Qamil.BGM", L"TooBigToFail.mp3", 136.f, -0.13f, 0.35f };
	}

	namespace HiFiRushSound
	{
		inline constexpr wchar_t TutorialCrowd[] = L"Tutorial.Crowd";
		inline constexpr wchar_t FightAnnouncement[] = L"UI.Fight";
		inline constexpr wchar_t BeatHitSuccess[] = L"UI.BeatHit.Success";
		inline constexpr wchar_t PipeHit[] = L"Environment.PipeHit";
		inline constexpr wchar_t OutsideWallTrigger[] = L"Outside.WallTrigger";
		inline constexpr wchar_t OutsideTriggerOpen[] = L"Outside.TriggerOpen";

		inline constexpr std::array ChiAttackVoices{ L"Chi.AttackVoice0", L"Chi.AttackVoice1", L"Chi.AttackVoice2", L"Chi.AttackVoice3", L"Chi.AttackVoice4" };
		inline constexpr std::array ChiJumpVoices{ L"Chi.Jump0", L"Chi.Jump1" };
		inline constexpr wchar_t ChiDash[] = L"Chi.Dash";
		inline constexpr std::array ChiFootsteps{ L"Chi.Footstep0", L"Chi.Footstep1", L"Chi.Footstep2", L"Chi.Footstep3" };
		inline constexpr wchar_t ChiDamage[] = L"Chi.Damage";
		inline constexpr std::array ChiStumps{ L"Chi.Stump0", L"Chi.Stump1", L"Chi.Stump2" };
		inline constexpr std::array ChiWeakSwings{ L"Chi.Swing.Weak0", L"Chi.Swing.Weak1", L"Chi.Swing.Weak2", L"Chi.Swing.Weak3" };
		inline constexpr std::array ChiStrongSwings{ L"Chi.Swing.Strong0", L"Chi.Swing.Strong1", L"Chi.Swing.Strong2" };
		inline constexpr wchar_t ChiLeftBranch[] = L"Chi.Branch.Left";
		inline constexpr wchar_t ChiRightBranch[] = L"Chi.Branch.Right";
		inline constexpr wchar_t ChiLeftBranchBeatHit[] = L"Chi.BeatHit.LeftBranch";
		inline constexpr wchar_t ChiRightBranchBeatHit[] = L"Chi.BeatHit.RightBranch";
		inline constexpr wchar_t ChiWeakBeatHit[] = L"Chi.BeatHit.Weak";
		inline constexpr wchar_t ChiStrongBeatHit[] = L"Chi.BeatHit.Strong";
		inline constexpr wchar_t ChiHibikiReady[] = L"Chi.Hibiki.Ready";
		inline constexpr wchar_t ChiHibikiAttack[] = L"Chi.Hibiki.Attack";
		inline constexpr wchar_t ChiHibikiBeatHit[] = L"Chi.Hibiki.BeatHit";

		inline constexpr std::array MonsterHitImpacts{ L"Monster.Hit0", L"Monster.Hit1", L"Monster.Hit2", L"Monster.Hit3" };
		inline constexpr std::array MonsterFootsteps{ L"Monster.Footstep0", L"Monster.Footstep1", L"Monster.Footstep2", L"Monster.Footstep3" };
		inline constexpr wchar_t SwordCharge[] = L"Sword.Charge";
		inline constexpr std::array SwordSwings{ L"Sword.Swing0", L"Sword.Swing1", L"Sword.Swing2" };
		inline constexpr std::array SwordDamageVoices{ L"Sword.Damage0", L"Sword.Damage1", L"Sword.Damage2", L"Sword.Damage3", L"Sword.Damage4", L"Sword.Damage5" };
		inline constexpr std::array SwordDeathVoices{ L"Sword.Death0", L"Sword.Death1", L"Sword.Death2" };
		inline constexpr std::array SwordDashes{ L"Sword.Dash0", L"Sword.Dash1", L"Sword.Dash2", L"Sword.Dash3" };
		inline constexpr wchar_t GunnerShoot[] = L"Gunner.Shoot";
		inline constexpr std::array GunnerDeathVoices{ L"Gunner.Death0", L"Gunner.Death1", L"Gunner.Death2" };
		inline constexpr std::array GunnerDashes{ L"Gunner.Dash0", L"Gunner.Dash1", L"Gunner.Dash2", L"Gunner.Dash3" };

		inline constexpr wchar_t QamilMove[] = L"Qamil.Move";
		inline constexpr wchar_t QamilChain[] = L"Qamil.Chain";
		inline constexpr wchar_t QamilSweep[] = L"Qamil.Sweep";
		inline constexpr wchar_t QamilStump[] = L"Qamil.Stump";
		inline constexpr wchar_t QamilLaser[] = L"Qamil.Laser";
		inline constexpr wchar_t QamilMissileExplosion[] = L"Qamil.MissileExplosion";
	}

	std::span<const SoundResourceDesc> GetGameplaySoundResources();
	std::span<const RhythmBGMDesc* const> GetRhythmBGMs();
	const RhythmBGMDesc*	GetCurrentRhythmBGM();
	float					GetCurrentRhythmBGMSongOffset();
	void PlayRandomSound2D(std::span<const wchar_t* const> resourceKeys, float volume = 1.f);
	void PlayRhythmBGM(const RhythmBGMDesc& desc);
	void PlayRhythmBGM(const RhythmBGMDesc& desc, float songOffsetSeconds);
	void TransitionRhythmBGM(const RhythmBGMDesc& desc);
	void SetCurrentRhythmBGMSongOffset(float songOffsetSeconds);
}
