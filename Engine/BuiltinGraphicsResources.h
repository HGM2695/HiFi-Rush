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

		// PS
		inline constexpr const wchar_t* FullScreenTexturePS = L"Engine.FullScreenTexturePS";
		inline constexpr const wchar_t* SpriteTexturePS = L"Engine.SpriteTexturePS";
		inline constexpr const wchar_t* SolidColorPS = L"Engine.SolidColorPS";
		inline constexpr const wchar_t* StaticMeshPS = L"Engine.StaticMeshPS";

		// Font
		inline constexpr const wchar_t* DefaultUIFont = L"Engine.DefaultUIFont";
	}

	class BuiltinGraphicsResources
	{
	public:
		static bool Load(Resources& resources, IGraphicsResourceFactory& factory);
	};
}
