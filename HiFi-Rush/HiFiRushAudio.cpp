#include "HiFiRushAudio.h"
#include "Application.h"
#include "AudioStatics.h"
#include "AudioSystem.h"
#include "BeatSystem.h"
#include "HiFiRushStatics.h"
#include "Resources.h"
#include "SoundWave.h"

#include <array>
#include <random>

namespace gm
{
	namespace
	{
		constexpr std::array<const RhythmBGMDesc*, 4> RhythmBGMs = { &HiFiRushBGM::Tutorial, &HiFiRushBGM::TutorialRoadUp, &HiFiRushBGM::Outside, &HiFiRushBGM::Qamil };
		const RhythmBGMDesc* CurrentRhythmBGM = nullptr;
		float CurrentRhythmBGMSongOffset = 0.f;
		float CurrentRhythmBGMTimelineOffset = 0.f;

		constexpr std::array GameplaySoundResources =
		{
			SoundResourceDesc{ HiFiRushSound::TutorialCrowd, L"Gameplay/amb_training_room_crowd_714.wav" },
			SoundResourceDesc{ HiFiRushSound::FightAnnouncement, L"Gameplay/Fight_John.wav" },
			SoundResourceDesc{ HiFiRushSound::BeatHitSuccess, L"Gameplay/HBKm_ST12_SP_Tapping_Success.wav" },
			SoundResourceDesc{ HiFiRushSound::PipeHit, L"Gameplay/SteamTriggerHit.mp3" },
			SoundResourceDesc{ HiFiRushSound::OutsideWallTrigger, L"Gameplay/WallTrigger.mp3" },
			SoundResourceDesc{ HiFiRushSound::OutsideTriggerOpen, L"TriggerOpen.mp3" },

			SoundResourceDesc{ HiFiRushSound::ChiAttackVoices[0], L"Gameplay/L_Attack0.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiAttackVoices[1], L"Gameplay/L_Attack1.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiAttackVoices[2], L"Gameplay/L_Attack2.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiAttackVoices[3], L"Gameplay/L_Attack3.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiAttackVoices[4], L"Gameplay/L_Attack4.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiJumpVoices[0], L"Gameplay/Jump0.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiJumpVoices[1], L"Gameplay/Jump1.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiDash, L"Gameplay/Dash.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiFootsteps[0], L"Gameplay/Run0.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiFootsteps[1], L"Gameplay/Run1.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiFootsteps[2], L"Gameplay/Run2.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiFootsteps[3], L"Gameplay/Run3.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiDamage, L"Gameplay/PlayerHitSound.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiStumps[0], L"Gameplay/Stump0.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiStumps[1], L"Gameplay/Stump1.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiStumps[2], L"Gameplay/Stump2.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiWeakSwings[0], L"Gameplay/ch0000_act_swish_norm_01_03.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiWeakSwings[1], L"Gameplay/ch0000_act_swish_norm_01_04.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiWeakSwings[2], L"Gameplay/ch0000_act_swish_norm_01_05.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiWeakSwings[3], L"Gameplay/ch0000_act_swish_norm_01_06.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiStrongSwings[0], L"Gameplay/ch0000_act_swish_hard_01_01.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiStrongSwings[1], L"Gameplay/ch0000_act_swish_hard_01_02.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiStrongSwings[2], L"Gameplay/ch0000_act_swish_hard_01_03.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiLeftBranch, L"Gameplay/LeftStart.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiRightBranch, L"Gameplay/RightStart.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiLeftBranchBeatHit, L"Gameplay/LeftStart_Beathit.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiRightBranchBeatHit, L"Gameplay/RightStart_Beathit.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiWeakBeatHit, L"Gameplay/WeakAttack_BeatHit.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiStrongBeatHit, L"Gameplay/StrongAttack_BeatHit.mp3" },
			SoundResourceDesc{ HiFiRushSound::ChiHibikiReady, L"Gameplay/tmpsnd_pl_com_atk_sp_hit_01.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiHibikiAttack, L"Gameplay/HBKm_ST06_PL_Ch0000_SP_InstFX_DBLRIFF_01.wav" },
			SoundResourceDesc{ HiFiRushSound::ChiHibikiBeatHit, L"Gameplay/HBKm_ST08_Pl_Ch0000_At_InstFX_BeatHits02.wav" },

			SoundResourceDesc{ HiFiRushSound::MonsterHitImpacts[0], L"Gameplay/Monsterhit0.mp3" },
			SoundResourceDesc{ HiFiRushSound::MonsterHitImpacts[1], L"Gameplay/Monsterhit1.mp3" },
			SoundResourceDesc{ HiFiRushSound::MonsterHitImpacts[2], L"Gameplay/Monsterhit2.mp3" },
			SoundResourceDesc{ HiFiRushSound::MonsterHitImpacts[3], L"Gameplay/Monsterhit3.mp3" },
			SoundResourceDesc{ HiFiRushSound::MonsterFootsteps[0], L"Gameplay/em0100_fs_walk_01_01.wav" },
			SoundResourceDesc{ HiFiRushSound::MonsterFootsteps[1], L"Gameplay/em0100_fs_walk_01_02.wav" },
			SoundResourceDesc{ HiFiRushSound::MonsterFootsteps[2], L"Gameplay/em0100_fs_walk_01_03.wav" },
			SoundResourceDesc{ HiFiRushSound::MonsterFootsteps[3], L"Gameplay/em0100_fs_walk_01_04.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordCharge, L"Gameplay/enm_em0100_atk_saber_charge_01.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordSwings[0], L"Gameplay/em0100_atk_saber_swish_l_01_04.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordSwings[1], L"Gameplay/em0100_atk_saber_swish_l_01_02.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordSwings[2], L"Gameplay/em0100_atk_saber_swish_l_01_03.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDamageVoices[0], L"Gameplay/enm_effo_saber_a_damage_01_1.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDamageVoices[1], L"Gameplay/enm_effo_saber_a_damage_01_2.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDamageVoices[2], L"Gameplay/enm_effo_saber_a_damage_01_3.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDamageVoices[3], L"Gameplay/enm_effo_saber_a_damage_01_4.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDamageVoices[4], L"Gameplay/enm_effo_saber_a_damage_01_5.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDamageVoices[5], L"Gameplay/enm_effo_saber_a_damage_01_6.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDeathVoices[0], L"Gameplay/enm_effo_saber_a_dead_01_1.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDeathVoices[1], L"Gameplay/enm_effo_saber_a_dead_01_2.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDeathVoices[2], L"Gameplay/enm_effo_saber_a_dead_01_3.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDashes[0], L"Gameplay/enm_efforts_tall_a_fastmove_01_1.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDashes[1], L"Gameplay/enm_efforts_tall_a_fastmove_01_2.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDashes[2], L"Gameplay/enm_efforts_tall_a_fastmove_01_3.wav" },
			SoundResourceDesc{ HiFiRushSound::SwordDashes[3], L"Gameplay/enm_efforts_tall_a_fastmove_01_4.wav" },
			SoundResourceDesc{ HiFiRushSound::GunnerShoot, L"Gameplay/Gunner_Shoot.mp3" },
			SoundResourceDesc{ HiFiRushSound::GunnerDeathVoices[0], L"Gameplay/enm_effo_saber_a_dead_01_4.wav" },
			SoundResourceDesc{ HiFiRushSound::GunnerDeathVoices[1], L"Gameplay/enm_effo_saber_a_dead_01_5.wav" },
			SoundResourceDesc{ HiFiRushSound::GunnerDeathVoices[2], L"Gameplay/enm_effo_saber_a_dead_01_6.wav" },
			SoundResourceDesc{ HiFiRushSound::GunnerDashes[0], L"Gameplay/enm_efforts_tall_a_fastmove_01_5.wav" },
			SoundResourceDesc{ HiFiRushSound::GunnerDashes[1], L"Gameplay/enm_efforts_tall_a_fastmove_01_6.wav" },
			SoundResourceDesc{ HiFiRushSound::GunnerDashes[2], L"Gameplay/enm_efforts_tall_a_fastmove_01_7.wav" },
			SoundResourceDesc{ HiFiRushSound::GunnerDashes[3], L"Gameplay/enm_efforts_tall_a_fastmove_01_8.wav" },

			SoundResourceDesc{ HiFiRushSound::QamilMove, L"Gameplay/Qamil_Move.mp3" },
			SoundResourceDesc{ HiFiRushSound::QamilChain, L"Gameplay/Chain.mp3" },
			SoundResourceDesc{ HiFiRushSound::QamilSweep, L"Gameplay/Sweep_qamil.mp3" },
			SoundResourceDesc{ HiFiRushSound::QamilStump, L"Gameplay/Qamil_Stump.mp3" },
			SoundResourceDesc{ HiFiRushSound::QamilLaser, L"Gameplay/Razer_1.mp3" },
			SoundResourceDesc{ HiFiRushSound::QamilMissileExplosion, L"Gameplay/Bomb.mp3" },
		};

		bool PlayBGM(const RhythmBGMDesc& desc)
		{
			const std::shared_ptr<SoundWave> sound = APPLICATION.GetResources().Find<SoundWave>(desc.commonResourceKey);
			GM_ASSERT_RETURN_VAL(sound, false, "BGM SoundWave 리소스가 존재하지 않습니다. key=%ls", desc.commonResourceKey);

			FMOD::Channel* channel = APPLICATION.GetAudioSystem().PlayBGM(*sound, desc.volume);
			GM_ASSERT_RETURN_VAL(channel, false, "BGM 재생에 실패했습니다. key=%ls", desc.commonResourceKey);
			return true;
		}

		void StartRhythmBGM(const RhythmBGMDesc& desc, float songOffsetSeconds, float timelineOffsetSeconds)
		{
			if (PlayBGM(desc) == false)
				return;

			CurrentRhythmBGM = &desc;
			CurrentRhythmBGMSongOffset = songOffsetSeconds;
			CurrentRhythmBGMTimelineOffset = timelineOffsetSeconds;
			HiFiRushStatics::GetBeatSystem().SetBPM(desc.bpm, timelineOffsetSeconds + songOffsetSeconds);
		}
	}

	std::span<const SoundResourceDesc> GetGameplaySoundResources()
	{
		return GameplaySoundResources;
	}

	std::span<const RhythmBGMDesc* const> GetRhythmBGMs()
	{
		return RhythmBGMs;
	}

	const RhythmBGMDesc* GetCurrentRhythmBGM()
	{
		return CurrentRhythmBGM;
	}

	float GetCurrentRhythmBGMSongOffset()
	{
		return CurrentRhythmBGMSongOffset;
	}

	void PlayRandomSound2D(std::span<const wchar_t* const> resourceKeys, float volume)
	{
		GM_ASSERT_RETURN(resourceKeys.empty() == false, "무작위 재생할 Sound Resource Key가 없습니다.");
		static std::mt19937 generator{ std::random_device{}() };
		std::uniform_int_distribution<size_t> distribution{ 0, resourceKeys.size() - 1 };
		PlaySound2D(resourceKeys[distribution(generator)], volume);
	}

	void PlayRhythmBGM(const RhythmBGMDesc& desc)
	{
		StartRhythmBGM(desc, desc.songOffsetSeconds, 0.f);
	}

	void PlayRhythmBGM(const RhythmBGMDesc& desc, float songOffsetSeconds)
	{
		StartRhythmBGM(desc, songOffsetSeconds, 0.f);
	}

	void TransitionRhythmBGM(const RhythmBGMDesc& desc)
	{
		GM_ASSERT_RETURN(desc.bpm > 0.f, "BGM BPM은 0보다 커야 합니다. key=%ls", desc.commonResourceKey);
		BeatSystem& beatSystem = HiFiRushStatics::GetBeatSystem();
		const float currentBeat = beatSystem.GetCurrentBeat();
		const float continuedBeatOffset = currentBeat * (60.f / desc.bpm);
		StartRhythmBGM(desc, desc.songOffsetSeconds, continuedBeatOffset);
	}

	void SetCurrentRhythmBGMSongOffset(float songOffsetSeconds)
	{
		GM_ASSERT_RETURN(CurrentRhythmBGM, "현재 재생 중인 Rhythm BGM이 없습니다.");
		CurrentRhythmBGMSongOffset = songOffsetSeconds;
		HiFiRushStatics::GetBeatSystem().SetBPM(CurrentRhythmBGM->bpm, CurrentRhythmBGMTimelineOffset + CurrentRhythmBGMSongOffset);
	}
}
