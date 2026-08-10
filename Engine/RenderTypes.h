#pragma once

#include "BoundingTypes.h"
#include "GraphicsTypes.h"
#include "MathTypes.h"
#include "SpriteFrame.h"
#include <cstddef>
#include <memory>
#include <vector>

namespace gm
{
	class Material;
	class Mesh;
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
		BoundingVolume					worldBounds{};
		const StaticMesh*				staticMesh = nullptr;
		std::vector<const Material*>	materials;
	};

	struct StaticMeshBatchKey
	{
		const Mesh*		mesh = nullptr;
		const Material*	material = nullptr;
		size_t			materialStateHash = 0;
		uint32			indexStart = 0;
		uint32			indexCount = 0;

		bool operator==(const StaticMeshBatchKey& rhs) const;
	};

	struct StaticMeshRenderBatch
	{
		StaticMeshBatchKey	key{};
		std::vector<Matrix>	worlds;
	};

	/// SkeletalMesh ////////////////////////////////////////////////////////////////////////////////////////////////
	struct SkeletalMeshRenderItem
	{
		Matrix								world = Matrix::CreateScale(1.f);
		BoundingVolume						worldBounds{};
		const SkeletalMesh*					skeletalMesh = nullptr;
		const std::vector<Matrix>*			boneModelMatrices = nullptr;
		std::vector<const Material*>		materials;
	};
}
