#pragma once

#include "EngineCore.h"

namespace gm
{
	class Resources;
	class IGraphicsResourceFactory;

	inline constexpr const wchar_t* FullScreenMesh = L"Engine.FullScreenMesh";
	inline constexpr const wchar_t* FullScreenTextureVS = L"Engine.FullScreenTextureVS";
	inline constexpr const wchar_t* FullScreenTexturePS = L"Engine.FullScreenTexturePS";
	inline constexpr const wchar_t* FullScreenPipelineState = L"Engine.FullScreenPipelineState";

	class BuiltinGraphicsResources
	{
	public:
		static bool Load(Resources& resources, IGraphicsResourceFactory& factory);
	};

}


