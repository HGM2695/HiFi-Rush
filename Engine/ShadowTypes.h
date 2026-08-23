#pragma once

#include "MathTypes.h"
#include <array>

namespace gm
{
	inline constexpr uint32 MaxShadowCascadeCount = 4;

	struct ShadowSettings
	{
		uint32	mapResolution = 2048;
		uint32	cascadeCount = 3;
		float	maxDistance = 67.5f;
		float	cascadeSplitLambda = 0.5f;
		float	depthBias = 0.001f;
		float	normalBias = 0.02f;
		uint32	pcfRadius = 1;
		float	shadowStrength = 1.f;
		Color	shadowColor = Color{ 0.35f, 0.47f, 0.71f, 1.f };
	};

	struct CascadedShadowRenderData
	{
		std::array<Matrix, MaxShadowCascadeCount>	viewProjections{};
		Vector4										splitDistances{};
		uint32										cascadeCount = 0;
		uint32										pcfRadius = 0;
		float										depthBias = 0.f;
		float										normalBias = 0.f;
		float										shadowMapTexelSize = 0.f;
		float										shadowStrength = 0.f;
		Vector2										padding0{};
		Vector3										shadowColor{};
		float										padding1 = 0.f;
	};

	static_assert(sizeof(CascadedShadowRenderData) % 16 == 0);
}
