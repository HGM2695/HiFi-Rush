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
	class SkeletalMesh;
	class Texture;
	
	/// Sprite ////////////////////////////////////////////////////////////////////////////////////////////////
	struct SpriteRenderItem
	{
		Matrix			world = Matrix::CreateScale(1.f);
		const Material* material = nullptr;
	};

	struct SpriteConstantPS
	{
		Vector2 uvOffset{ 0.f, 0.f };
		Vector2 uvScale{ 1.f, 1.f };
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

	/// SkeletalMesh ////////////////////////////////////////////////////////////////////////////////////////////////
	struct SkeletalMeshRenderItem
	{
		Matrix								world = Matrix::CreateScale(1.f);
		const SkeletalMesh*					skeletalMesh = nullptr;
		const std::vector<Matrix>*			boneModelMatrices = nullptr;
		std::vector<const Material*>		materials;
	};
}
