#pragma once

#include "MathTypes.h"
#include "MonsterStateContext.h"
#include "MonsterStateTypes.h"

#include <array>
#include <string>

namespace gm
{
	struct HitEvent;

	inline constexpr float MonsterDefaultBlendDuration = 0.15f;

	class MonsterState
	{
	public:
		virtual ~MonsterState() = default;

		virtual MonsterStateId GetStateId() const = 0;
		virtual void Enter(MonsterStateContext& context) {}
		virtual void Tick(MonsterStateContext& context, float deltaTime) {}
		virtual void OnDamaged(MonsterStateContext& context, const HitEvent& event);
		virtual void OnGroundContact(MonsterStateContext& context) {}
		virtual void Exit(MonsterStateContext& context) {}

	protected:
		void FaceTarget(MonsterStateContext& context, float deltaTime) const;
		void FaceTargetImmediate(MonsterStateContext& context) const;
		void SetRootMotionEnabled(MonsterStateContext& context, bool enabled) const;
		bool PlayAnimation(MonsterStateContext& context, const std::wstring& clipName, bool isLoop, float blendDuration = MonsterDefaultBlendDuration) const;
		bool IsAnimationCompleted(const MonsterStateContext& context) const;
	};

	class MonsterAirborneState final : public MonsterState
	{
	public:
		MonsterAirborneState(std::wstring launchClipName, std::wstring fallClipName, std::array<std::wstring, 3> hitClipNames);

		MonsterStateId GetStateId() const override { return MonsterStateId::Airborne; }
		void Enter(MonsterStateContext& context) override;
		void Tick(MonsterStateContext& context, float deltaTime) override;
		void OnDamaged(MonsterStateContext& context, const HitEvent& event) override;
		void OnGroundContact(MonsterStateContext& context) override;
		void Exit(MonsterStateContext& context) override;

	private:
		void SaveMotionSettings(MonsterStateContext& context);
		void RestoreMotionSettings(MonsterStateContext& context);
		void SetRootMotion(MonsterStateContext& context, bool enabled, const Vector3& weight) const;
		void PlayAirHitAnimation(MonsterStateContext& context, bool applyRootMotionY);
		bool HasReachedSkyHitFallTransition(const MonsterStateContext& context) const;
		void BeginFall(MonsterStateContext& context);

	private:
		enum class Phase
		{
			Launch,
			Hit,
			Fall,
		};

		std::wstring				_launchClipName;
		std::wstring				_fallClipName;
		std::array<std::wstring, 3>	_hitClipNames;
		Phase						_phase = Phase::Launch;
		Vector3						_previousRootMotionWeight{ 1.f, 1.f, 1.f };
		bool						_previousRootMotionEnabled = true;
		bool						_previousUseGravity = true;
		bool						_isSkyHit = false;
		bool						_hasSavedMotionSettings = false;
	};

	class MonsterDownState final : public MonsterState
	{
	public:
		explicit MonsterDownState(std::wstring clipName);

		MonsterStateId GetStateId() const override { return MonsterStateId::Down; }
		void Enter(MonsterStateContext& context) override;
		void Tick(MonsterStateContext& context, float deltaTime) override;

	private:
		std::wstring _clipName;
	};

	class MonsterWakeUpState final : public MonsterState
	{
	public:
		explicit MonsterWakeUpState(std::wstring clipName);

		MonsterStateId GetStateId() const override { return MonsterStateId::WakeUp; }
		void Enter(MonsterStateContext& context) override;
		void Tick(MonsterStateContext& context, float deltaTime) override;

	private:
		std::wstring _clipName;
	};
}
