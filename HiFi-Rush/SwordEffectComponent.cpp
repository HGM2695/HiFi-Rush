#include "SwordEffectComponent.h"

#include "EffectPresets.h"
#include "GameObject.h"
#include "Resources.h"
#include "Scene.h"
#include "SwordAnimationTypes.h"

namespace gm
{
	namespace
	{
		constexpr wchar_t SlashChargeEffectId[] = L"Sword.Attack.Slash.Charge";
		constexpr wchar_t SlashEffectId[] = L"Sword.Attack.Slash.Swing";
		constexpr wchar_t JumpSlashChargeEffectId[] = L"Sword.Attack.JumpSlash.Charge";
		constexpr wchar_t JumpSlashEffectId[] = L"Sword.Attack.JumpSlash.Swing";

		const wchar_t* GetChargeEffectId(SwordAnimationId animationId)
		{
			switch (animationId)
			{
			case SwordAnimationId::AttackSlash:
				return SlashChargeEffectId;
			case SwordAnimationId::AttackJumpSlash:
				return JumpSlashChargeEffectId;
			default:
				GM_ASSERT_RETURN_VAL(false, nullptr, "Sword Charge Effect를 지원하지 않는 Animation입니다.");
			}
		}

		const wchar_t* GetSlashEffectId(SwordAnimationId animationId)
		{
			switch (animationId)
			{
			case SwordAnimationId::AttackSlash:
				return SlashEffectId;
			case SwordAnimationId::AttackJumpSlash:
				return JumpSlashEffectId;
			default:
				GM_ASSERT_RETURN_VAL(false, nullptr, "Sword Slash Effect를 지원하지 않는 Animation입니다.");
			}
		}
	}

	SwordEffectComponent::SwordEffectComponent(Resources& resources, const EffectPresets& effectPresets)
		: _effectSpawner(resources, effectPresets)
	{
	}

	bool SwordEffectComponent::SpawnChargeEffect(SwordAnimationId animationId)
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN_VAL(scene, false, "Sword Charge Effect를 생성할 Scene이 없습니다.");
		const wchar_t* effectId = GetChargeEffectId(animationId);
		return effectId && _effectSpawner.SpawnAttachedToOwner(*scene, effectId, GetOwner());
	}

	bool SwordEffectComponent::SpawnSlashEffect(SwordAnimationId animationId)
	{
		Scene* scene = GetOwner().GetScene();
		GM_ASSERT_RETURN_VAL(scene, false, "Sword Slash Effect를 생성할 Scene이 없습니다.");
		const wchar_t* effectId = GetSlashEffectId(animationId);
		return effectId && _effectSpawner.SpawnAttachedToOwner(*scene, effectId, GetOwner());
	}
}
