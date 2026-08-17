#pragma once

#include "MonsterStateContext.h"
#include "MonsterStateTypes.h"

#include <array>
#include <string>

namespace gm
{
	struct HitEvent;
	struct NavigationGroundContactEvent;

	inline constexpr float MonsterDefaultBlendDuration = 0.15f;

	class MonsterState
	{
	public:
		virtual ~MonsterState() = default;

		virtual MonsterStateId GetStateId() const = 0;
		virtual void Enter(MonsterStateContext& context) {}
		virtual void Tick(MonsterStateContext& context, float deltaTime) {}
		virtual void OnDamaged(MonsterStateContext& context, const HitEvent& event);
		virtual void OnGroundContact(MonsterStateContext& context, const NavigationGroundContactEvent& event) {}
		virtual void Exit(MonsterStateContext& context) {}

	protected:
		void FaceTarget(MonsterStateContext& context, float deltaTime) const;
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
		void OnGroundContact(MonsterStateContext& context, const NavigationGroundContactEvent& event) override;

	private:
		void Launch(MonsterStateContext& context, float speed) const;
		void PlayAirHitAnimation(MonsterStateContext& context);

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
		Phase					_phase = Phase::Launch;
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
