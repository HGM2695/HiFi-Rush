#pragma once

#include "EngineCore.h"

namespace gm
{
	class Resources;
	class IGraphicsResourceFactory;

	inline constexpr const wchar_t* FullScreenMesh = L"Engine.FullscreenMesh";
	inline constexpr const wchar_t* FullScreenTextureVS = L"Engine.FullscreenTextureVS";
	inline constexpr const wchar_t* FullScreenTexturePS = L"Engine.FullscreenTexturePS";

	class BuiltinGraphicsResources
	{
	public:
		static bool Load(Resources& resources, IGraphicsResourceFactory& factory);
	};

}


