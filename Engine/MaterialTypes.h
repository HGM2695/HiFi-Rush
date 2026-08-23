#pragma once

#include "MathTypes.h"

namespace gm
{
	enum class ShadingModel
	{
		Unlit = 0,
		ToonLit = 1,
		Lit = 2,
		Count
	};

	enum class SurfaceMode
	{
		Opaque,
		Masked,
		Transparent,
		Count
	};

	enum class OutlineMode
	{
		Disabled,
		Enabled,
		Count
	};

	enum class MaterialColorMode
	{
		None,
		Blend,
		OpacityGradient,
		Count
	};

	struct MaterialSurfaceData
	{
		ShadingModel	shadingModel = ShadingModel::Unlit;
		SurfaceMode		surfaceMode = SurfaceMode::Opaque;
		OutlineMode	outlineMode = OutlineMode::Enabled;
		Color			emissiveColor = Colors::White;
		float			emissiveIntensity = 0.f;
		float			alphaCutoff = 0.5f;

		bool operator==(const MaterialSurfaceData& rhs) const
		{
			return shadingModel == rhs.shadingModel && surfaceMode == rhs.surfaceMode && outlineMode == rhs.outlineMode &&
				emissiveColor.x == rhs.emissiveColor.x && emissiveColor.y == rhs.emissiveColor.y &&
				emissiveColor.z == rhs.emissiveColor.z && emissiveColor.w == rhs.emissiveColor.w &&
				emissiveIntensity == rhs.emissiveIntensity && alphaCutoff == rhs.alphaCutoff;
		}
	};

	struct MaterialColorData
	{
		MaterialColorMode	mode = MaterialColorMode::None;
		Color				blendColor = Colors::White;
		Color				opacityLowColor = Colors::White;
		Color				opacityHighColor = Colors::White;
		Color				colorMultiplier = Colors::White;
		float				blendRatio = 0.f;

		bool operator==(const MaterialColorData& rhs) const
		{
			return mode == rhs.mode && blendColor.x == rhs.blendColor.x && blendColor.y == rhs.blendColor.y &&
				blendColor.z == rhs.blendColor.z && blendColor.w == rhs.blendColor.w &&
				opacityLowColor.x == rhs.opacityLowColor.x && opacityLowColor.y == rhs.opacityLowColor.y &&
				opacityLowColor.z == rhs.opacityLowColor.z && opacityLowColor.w == rhs.opacityLowColor.w &&
				opacityHighColor.x == rhs.opacityHighColor.x && opacityHighColor.y == rhs.opacityHighColor.y &&
				opacityHighColor.z == rhs.opacityHighColor.z && opacityHighColor.w == rhs.opacityHighColor.w &&
				colorMultiplier.x == rhs.colorMultiplier.x && colorMultiplier.y == rhs.colorMultiplier.y &&
				colorMultiplier.z == rhs.colorMultiplier.z && colorMultiplier.w == rhs.colorMultiplier.w &&
				blendRatio == rhs.blendRatio;
		}
	};
}
