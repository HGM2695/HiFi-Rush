#include "Random.h"
#include "GMAssert.h"

#include <mutex>
#include <random>

namespace
{
	struct RandomState
	{
		std::mt19937 generator{ std::random_device{}() };
		std::mutex mutex;
	};

	RandomState& GetRandomState()
	{
		static RandomState state;
		return state;
	}
}

namespace gm::Math
{
	void SetRandomSeed(uint32 seed)
	{
		RandomState& state = GetRandomState();
		const std::scoped_lock lock{ state.mutex };
		state.generator.seed(seed);
	}

	int32 RandomInt(int32 minInclusive, int32 maxInclusive)
	{
		GM_ASSERT_RETURN_VAL(minInclusive <= maxInclusive, minInclusive, "RandomInt의 최솟값은 최댓값보다 클 수 없습니다.");

		RandomState& state = GetRandomState();
		const std::scoped_lock lock{ state.mutex };
		return std::uniform_int_distribution<int32>{ minInclusive, maxInclusive }(state.generator);
	}

	float RandomFloat(float minValue, float maxValue)
	{
		GM_ASSERT_RETURN_VAL(minValue <= maxValue, minValue, "RandomFloat의 최솟값은 최댓값보다 클 수 없습니다.");

		RandomState& state = GetRandomState();
		const std::scoped_lock lock{ state.mutex };
		return std::uniform_real_distribution<float>{ minValue, maxValue }(state.generator);
	}
}
