#pragma once

namespace gm
{
	enum class ChiAnimationClipId;
	struct ChiStateContext;

	class ChiAttackHitBoxSpawner
	{
	public:
		static bool IsSpawnedByHitStartNotify(ChiAnimationClipId animationClipId);
		static bool SpawnForAnimation(ChiStateContext& context, ChiAnimationClipId animationClipId);

	private:
		ChiAttackHitBoxSpawner() = delete;
	};
}
