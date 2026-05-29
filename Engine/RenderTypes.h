#pragma once

#include "GraphicsTypes.h"
#include "MathTypes.h"
#include "SpriteFrame.h"
#include <memory>
#include <vector>

namespace gm
{
	class Material;
	class StaticMesh;
	class Texture;
	
	/// Sprite ////////////////////////////////////////////////////////////////////////////////////////////////
	struct SpriteRenderItem
	{
		Matrix			world = Matrix::CreateScale(1.f);
		const Material* material = nullptr;
	};

	struct SpriteConstantPS
	{
		float textureLeft = 0.f;
		float textureTop = 0.f;
		float textureWidth = 1.f;
		float textureHeight = 1.f;
	};

	struct ColorConstantPS
	{
		Color	color = Colors::White;
	};

	/// UI ////////////////////////////////////////////////////////////////////////////////////////////////
	struct UIRenderItem
	{
		Vector2			screenCenter{};
		Vector2			size{};
		const Material*	material = nullptr;
	};

	/// StaticMesh ////////////////////////////////////////////////////////////////////////////////////////////////
	struct StaticMeshRenderItem
	{
		Matrix							world = Matrix::CreateScale(1.f);
		const StaticMesh*				staticMesh = nullptr;
		std::vector<const Material*>	materials;
	};
}
