#pragma once

namespace gm
{
	enum class ChiAnimationClipId;
	struct BeatHitResultEvent;
	struct ChiStateContext;

	class ChiBeatHitBoxSpawner final
	{
	public:
		static bool Spawn(ChiStateContext& context, ChiAnimationClipId animationClipId, const BeatHitResultEvent& resultEvent);

	private:
		ChiBeatHitBoxSpawner() = delete;
	};
}
