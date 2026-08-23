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
		inline constexpr const wchar_t* StaticMeshVS = L"Engine.StaticMeshVS";
		inline constexpr const wchar_t* StaticMeshInstancedVS = L"Engine.StaticMeshInstancedVS";
		inline constexpr const wchar_t* SkeletalMeshVS = L"Engine.SkeletalMeshVS";
		inline constexpr const wchar_t* StaticShadowVS = L"Engine.StaticShadowVS";
		inline constexpr const wchar_t* StaticInstancedShadowVS = L"Engine.StaticInstancedShadowVS";
		inline constexpr const wchar_t* SkeletalShadowVS = L"Engine.SkeletalShadowVS";
		inline constexpr const wchar_t* SkySphereVS = L"Engine.SkySphereVS";

		// PS
		inline constexpr const wchar_t* FullScreenTexturePS = L"Engine.FullScreenTexturePS";
		inline constexpr const wchar_t* SpriteTexturePS = L"Engine.SpriteTexturePS";
		inline constexpr const wchar_t* EffectSpritePS = L"Engine.EffectSpritePS";
		inline constexpr const wchar_t* EffectMeshPS = L"Engine.EffectMeshPS";
		inline constexpr const wchar_t* SolidColorPS = L"Engine.SolidColorPS";
		inline constexpr const wchar_t* MeshForwardPS = L"Engine.MeshForwardPS";
		inline constexpr const wchar_t* UVBarGraphPS = L"Engine.UVBarGraphPS";
		inline constexpr const wchar_t* MeshGBufferPS = L"Engine.MeshGBufferPS";
		inline constexpr const wchar_t* DeferredCompositionPS = L"Engine.DeferredCompositionPS";
		inline constexpr const wchar_t* ScreenSpaceOutlinePS = L"Engine.ScreenSpaceOutlinePS";
		inline constexpr const wchar_t* DepthFogPS = L"Engine.DepthFogPS";
		inline constexpr const wchar_t* BloomDownsamplePS = L"Engine.BloomDownsamplePS";
		inline constexpr const wchar_t* BloomUpsamplePS = L"Engine.BloomUpsamplePS";
		inline constexpr const wchar_t* BloomCompositePS = L"Engine.BloomCompositePS";
		inline constexpr const wchar_t* ToneMappingPS = L"Engine.ToneMappingPS";
		inline constexpr const wchar_t* FXAAPS = L"Engine.FXAAPS";
		inline constexpr const wchar_t* SSAOPS = L"Engine.SSAOPS";
		inline constexpr const wchar_t* SSAODownsampleBlurPS = L"Engine.SSAODownsampleBlurPS";
		inline constexpr const wchar_t* SSAOUpsampleBlurPS = L"Engine.SSAOUpsampleBlurPS";
		inline constexpr const wchar_t* RenderTargetDebugPS = L"Engine.RenderTargetDebugPS";
		inline constexpr const wchar_t* MaskedShadowPS = L"Engine.MaskedShadowPS";
		inline constexpr const wchar_t* SkySpherePS = L"Engine.SkySpherePS";

		// Font
		inline constexpr const wchar_t* DefaultUIFont = L"Engine.DefaultUIFont";
	}

	class BuiltinGraphicsResources
	{
	public:
		static bool Load(Resources& resources, IGraphicsResourceFactory& factory);
	};
}
