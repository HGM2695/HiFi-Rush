#pragma once

#include "ShadowTypes.h"
#include <array>
#include <memory>

namespace gm
{
	class IGraphicsResourceFactory;
	class Texture;
	struct CameraViewInfo;

	struct ShadowCascade
	{
		Matrix				viewProjection = Matrix::Identity;
		BoundingOrientedBox casterBounds{};
		float				nearDistance = 0.f;
		float				farDistance = 0.f;
	};

	class CascadedShadowMap
	{
	public:
		bool							Initialize(IGraphicsResourceFactory& resourceFactory, const ShadowSettings& settings);
		void							UpdateSettings(const ShadowSettings& settings);
		void							UpdateCascades(const CameraViewInfo& viewInfo, const Vector3& lightDirection);
		void							ClearCascades();
		Texture&						GetTexture() { return *_texture; }
		const Texture&					GetTexture() const { return *_texture; }
		const ShadowCascade&			GetCascade(uint32 cascadeIndex) const { return _cascades[cascadeIndex]; }
		const CascadedShadowRenderData&	GetRenderData() const { return _renderData; }

	private:
		std::unique_ptr<Texture>						_texture;
		ShadowSettings									_settings{};
		std::array<ShadowCascade, MaxShadowCascadeCount>	_cascades{};
		CascadedShadowRenderData						_renderData{};
	};
}
