#pragma once

#include "MathTypes.h"
#include <array>

namespace gm
{
	inline constexpr uint32 MaxDirectionalLightCount = 4;
	inline constexpr uint32 MaxPointLightCount = 32;
	inline constexpr uint32 MaxSpotLightCount = 16;

	struct DirectionalLightRenderData
	{
		Vector3 direction{};
		float	padding = 0.f;
		Vector3 color{};
		float	intensity = 0.f;
	};

	struct PointLightRenderData
	{
		Vector3 position{};
		float	range = 0.f;
		Vector3 color{};
		float	intensity = 0.f;
	};

	struct  SpotLightRenderData
	{
		Vector3 position{};
		float	range = 0.f;
		Vector3 direction{};
		float	innerConeCosine = 0.f;
		Vector3 color{};
		float	intensity = 0.f;
		float	outerConeCosine = 0.f;
		uint32	coneFalloff = 0;
		uint32	cookieEnabled = 0;
		uint32	padding = 0;
		Vector3 right{};
		float	rightPadding = 0.f;
	};

	struct LightRenderData
	{
		Vector3 ambientColor{};
		float	ambientIntensity = 0.f;
		uint32	directionalLightCount = 0;
		uint32	pointLightCount = 0;
		uint32	spotLightCount = 0;
		int32	shadowDirectionalLightIndex = -1;
		std::array<DirectionalLightRenderData, MaxDirectionalLightCount>	directionalLights{};
		std::array<PointLightRenderData, MaxPointLightCount>				pointLights{};
		std::array<SpotLightRenderData, MaxSpotLightCount>					spotLights{};
	};

	static_assert(sizeof(DirectionalLightRenderData) % 16 == 0);
	static_assert(sizeof(PointLightRenderData) % 16 == 0);
	static_assert(sizeof(SpotLightRenderData) % 16 == 0);
	static_assert(sizeof(LightRenderData) % 16 == 0);
}
