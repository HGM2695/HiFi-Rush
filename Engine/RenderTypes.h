#pragma once

#include "BoundingTypes.h"
#include "GraphicsTypes.h"
#include "LightTypes.h"
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

	/// Light ////////////////////////////////////////////////////////////////////////////////////////////////
	struct LightRenderItem
	{
		LightType		lightType = LightType::Directional;
		Vector3			position{};
		Vector3			direction = Vector3::Forward;
		Vector3			right = Vector3::Right;
		Color			color = Colors::White;
		float			intensity = 1.f;
		float			range = 0.f;
		float			innerConeRadian = 0.f;
		float			outerConeRadian = 0.f;
		SpotConeFalloff spotConeFalloff = SpotConeFalloff::Smooth;
		bool			useSpotCookie = false;
		bool			castsShadow = false;
	};
	
	/// Sprite ////////////////////////////////////////////////////////////////////////////////////////////////
	enum class SpriteFacingMode
	{
		None,
		Billboard,
		YAxisBillboard,
		FixedUpAxisBillboard,
		FixedRightAxisBillboard,

		Count
	};

	struct SpriteRenderItem
	{
		Matrix				world = Matrix::CreateScale(1.f);
		const Material*		material = nullptr;
		SpriteFacingMode	facingMode = SpriteFacingMode::None;
		float				cameraDepth = 0.f;
		float				sortDepthOffset = 0.f;
		uint64				submissionOrder = 0;
	};

	struct SpriteConstantPS
	{
		Vector2 uvOffset{ 0.f, 0.f };
		Vector2 uvScale{ 1.f, 1.f };
		Color	blendColor = Colors::White;
		float	blendRatio = 0.f;
		float	opacity = 1.f;
		float	fillRatio = 1.f;
		uint32	fillMode = 0;
		float	radialStartAngle = 0.f;
		float	radialSweepAngle = 0.f;
		Vector2 radialCenter{ 0.5f, 0.5f };
	};

	struct ColorConstantPS
	{
		Color	color = Colors::White;
	};

	struct EffectMaterialConstantPS
	{
		Color	emissiveColor = Colors::White;
		float	dissolveThreshold = 0.f;
		float	emissiveIntensity = 0.f;
		uint32	dissolveEnabled = 0;
		float	padding = 0.f;
	};

	struct MaterialSurfaceConstantPS
	{
		uint32	shadingModel = 0;
		uint32	surfaceMode = 0;
		uint32	outlineMode = 0;
		float	emissiveIntensity = 0.f;
		float	alphaCutoff = 0.5f;
		Vector3	surfacePadding{};
		Color	emissiveColor = Colors::White;
		uint32	colorMode = 0;
		uint32	textureFlags = 0;
		float	colorBlendRatio = 0.f;
		float	colorPadding = 0.f;
		Color	blendColor = Colors::White;
		Color	opacityLowColor = Colors::White;
		Color	opacityHighColor = Colors::White;
		Color	colorMultiplier = Colors::White;
		Vector2 textureUVOffset{};
		Vector2 textureUVPadding{};
	};

	/// UI ////////////////////////////////////////////////////////////////////////////////////////////////
	struct UIRenderItem
	{
		Vector2			screenCenter{};
		Vector2			size{};
		float			rotation = 0.f;
		const Material*	material = nullptr;
	};

	/// StaticMesh ////////////////////////////////////////////////////////////////////////////////////////////////
	struct StaticMeshRenderItem
	{
		Matrix							world = Matrix::CreateScale(1.f);
		BoundingVolume					worldBounds{};
		const StaticMesh*				staticMesh = nullptr;
		std::vector<const Material*>	materials;
		uint64							submissionOrder = 0;
		bool							castsShadow = true;
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
		float				cameraDepth = 0.f;
		uint64				submissionOrder = 0;
	};

	/// SkeletalMesh ////////////////////////////////////////////////////////////////////////////////////////////////
	struct SkeletalMeshRenderItem
	{
		Matrix								world = Matrix::CreateScale(1.f);
		BoundingVolume						worldBounds{};
		const SkeletalMesh*					skeletalMesh = nullptr;
		const std::vector<Matrix>*			boneModelMatrices = nullptr;
		std::vector<const Material*>		materials;
		uint64								submissionOrder = 0;
		bool								castsShadow = true;
	};

	enum class TransparentRenderSource
	{
		Static,
		Skeletal,
		Sprite
	};

	struct TransparentRenderEntry
	{
		TransparentRenderSource	source = TransparentRenderSource::Static;
		uint32					itemIndex = 0;
		float					cameraDepth = 0.f;
		uint64					submissionOrder = 0;
	};

	enum class RenderTargetDebugView
	{
		OriginalScene,
		BaseColor,
		WorldNormal,
		AmbientOcclusion,
		ScreenSpaceAmbientOcclusion,
		ScreenSpaceOutline,
		MaterialFlags,
		Emissive,
		SceneDepth,
		BloomContribution,
		SceneColorA,
		SceneColorB,

		Count
	};
}
