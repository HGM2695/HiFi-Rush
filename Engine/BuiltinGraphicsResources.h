#pragma once

#include "EngineCore.h"

namespace gm
{
	class Resources;
	class IGraphicsResourceFactory;

	namespace BuiltinResourceKey
	{
		// Mesh
		inline constexpr const wchar_t* FullScreenMesh = L"Engine.FullScreenMesh";
		inline constexpr const wchar_t* UnitQuadMesh = L"Engine.UnitQuadMesh";

		// VS
		inline constexpr const wchar_t* FullScreenTextureVS = L"Engine.FullScreenTextureVS";
		inline constexpr const wchar_t* QuadVS = L"Engine.QuadVS";

		// PS
		inline constexpr const wchar_t* FullScreenTexturePS = L"Engine.FullScreenTexturePS";
		inline constexpr const wchar_t* SpriteTexturePS = L"Engine.SpriteTexturePS";
		inline constexpr const wchar_t* SolidColorPS = L"Engine.SolidColorPS";

		// PSO
		inline constexpr const wchar_t* FullScreenTexturePSO = L"Engine.FullScreenTexturePSO";
		inline constexpr const wchar_t* SpriteTexturePSO = L"Engine.SpriteTexturePSO";
		inline constexpr const wchar_t* SolidColorPSO = L"Engine.SolidColorPSO";

		// Sampler
		inline constexpr const wchar_t* PointSampler = L"Engine.PointSampler";
		inline constexpr const wchar_t* LinearSampler = L"Engine.LinearSampler";
	}

	class BuiltinGraphicsResources
	{
	public:
		static bool Load(Resources& resources, IGraphicsResourceFactory& factory);
	};
}
