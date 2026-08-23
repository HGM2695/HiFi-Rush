#pragma once

#include "EngineCore.h"
#include "CascadedShadowMap.h"
#include "FogTypes.h"
#include "GBuffer.h"
#include "LightRenderData.h"
#include "PingPongRenderTarget.h"
#include "RenderTypes.h"
#include "ShadowTypes.h"

namespace gm
{
	class IGraphicsCommandContext;
	class IGraphicsResourceFactory;
	class Resources;
	class BloomRenderPass;
	class DeferredCompositionPass;
	class DepthFogPass;
	class FullscreenRenderPass;
	class FXAARenderPass;
	class SpriteRenderPass;
	class StaticMeshRenderPass;
	class SkeletalMeshRenderPass;
	class ShadowCasterPass;
	class SSAORenderPass;
	class ScreenSpaceOutlinePass;
	class SkySphereRenderPass;
	class StaticMesh;
	class ToneMappingPass;
	class UIRenderPass;
	struct BloomSettings;
	struct SSAOSettings;
	struct ScreenSpaceOutlineSettings;
#if GM_ENABLE_DEBUG_TOOLS
	class IDebugRenderer;
	class RenderTargetDebugPass;

	struct MeshCullingDebugStats
	{
		uint32 submittedCount = 0;
		uint32 visibleCount = 0;
		uint32 culledCount = 0;
	};

	struct CullingDebugStats
	{
		MeshCullingDebugStats staticMesh{};
		MeshCullingDebugStats skeletalMesh{};
	};

	struct StaticMeshInstancingDebugStats
	{
		uint32 renderBatchCount = 0;
		uint32 normalDrawCallCount = 0;
		uint32 instancedDrawCallCount = 0;
		uint32 instancedInstanceCount = 0;
	};

#endif

	struct SpriteRenderItem;
	struct StaticMeshRenderItem;
	struct SkeletalMeshRenderItem;
	struct ToneMappingSettings;
	struct UIRenderItem;
	struct CameraViewInfo;

	class Renderer
	{
	public:
		Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory);
		~Renderer();

		bool Initialize(uint32 width, uint32 height);
		void SubmitLight(const LightRenderItem& item);
		void SubmitSprite(const SpriteRenderItem& item);
		void SubmitStaticMesh(const StaticMeshRenderItem& item);
		void SubmitSkeletalMesh(const SkeletalMeshRenderItem& item);
		void SubmitUI(const UIRenderItem& item);
		void SetSkySphere(const std::shared_ptr<StaticMesh>& staticMesh, const std::shared_ptr<Texture>& texture);
		void ClearSkySphere();
		void SetSpotLightCookieTexture(const std::shared_ptr<Texture>& texture) { _spotLightCookieTexture = texture; }
		const ShadowSettings& GetShadowSettings() const { return _shadowSettings; }
		const CascadedShadowMap& GetCascadedShadowMap() const { return _cascadedShadowMap; }
		void SetBloomSettings(const BloomSettings& settings);
		const BloomSettings& GetBloomSettings() const;
		void SetSSAOSettings(const SSAOSettings& settings);
		const SSAOSettings& GetSSAOSettings() const;
		void SetScreenSpaceOutlineSettings(const ScreenSpaceOutlineSettings& settings);
		const ScreenSpaceOutlineSettings& GetScreenSpaceOutlineSettings() const;

#if GM_ENABLE_DEBUG_TOOLS
		void							SetBoundingVolumeDebugDrawEnabled(bool isEnabled) { _isBoundingVolumeDebugDrawEnabled = isEnabled; }
		bool							IsBoundingVolumeDebugDrawEnabled() const { return _isBoundingVolumeDebugDrawEnabled; }
		void							SetFrustumCullingEnabled(bool isEnabled) { _isFrustumCullingEnabled = isEnabled; }
		bool							IsFrustumCullingEnabled() const { return _isFrustumCullingEnabled; }
		void							SetStaticMeshInstancingEnabled(bool isEnabled) { _isStaticMeshInstancingEnabled = isEnabled; }
		bool							IsStaticMeshInstancingEnabled() const { return _isStaticMeshInstancingEnabled; }
		void							SetScreenSpaceOutlineEnabled(bool isEnabled) { _isScreenSpaceOutlineEnabled = isEnabled; }
		bool							IsScreenSpaceOutlineEnabled() const { return _isScreenSpaceOutlineEnabled; }
		void							SetFXAAEnabled(bool isEnabled) { _isFXAAEnabled = isEnabled; }
		bool							IsFXAAEnabled() const { return _isFXAAEnabled; }
		void							SetRenderTargetDebugView(RenderTargetDebugView view) { _renderTargetDebugView = view; }
		RenderTargetDebugView			GetRenderTargetDebugView() const { return _renderTargetDebugView; }
		void							SelectNextRenderTargetDebugView();
		bool							SetShadowResolution(uint32 resolution);
		void							SetShadowCascadeCount(uint32 cascadeCount);
		void							SetShadowPCFRadius(uint32 pcfRadius);
		void							SetShadowStrength(float strength);
		void							SetShadowCascadeSplitLambda(float lambda);
		void							SetShadowMaxDistance(float distance);
		void							SetShadowDepthBias(float bias);
		void							SetShadowNormalBias(float bias);
		void							SetShadowColor(const Color& color);
		void							SetCascadeDebugColorsEnabled(bool isEnabled) { _showCascadeDebugColors = isEnabled; }
		bool							IsCascadeDebugColorsEnabled() const { return _showCascadeDebugColors; }
		CullingDebugStats				GetCullingDebugStats() const;
		StaticMeshInstancingDebugStats	GetStaticMeshInstancingDebugStats() const;
		void							DebugDraw(IDebugRenderer& debugRenderer) const;
#endif

		void Render(const SceneAmbientSettings& ambientSettings, const DepthFogSettings& depthFogSettings, const ToneMappingSettings& toneMappingSettings, const CameraViewInfo& viewInfo, uint32 width, uint32 height, const Color& clearColor);

		void Clear();

	private:
		bool CreateSceneRenderTargets(uint32 width, uint32 height);
		LightRenderData CollectLightRenderData(const SceneAmbientSettings& ambientSettings, const CameraViewInfo& viewInfo, const BoundingFrustum& worldFrustum) const;
#if GM_ENABLE_DEBUG_TOOLS
		const Texture& GetRenderTargetDebugTexture() const;
#endif

	private:
		IGraphicsCommandContext&					_commandContext;
		IGraphicsResourceFactory&					_resourceFactory;
		std::unique_ptr<FullscreenRenderPass>		_fullscreenRenderPass;
		std::unique_ptr<BloomRenderPass>			_bloomRenderPass;
		std::unique_ptr<ToneMappingPass>			_toneMappingPass;
		std::unique_ptr<FXAARenderPass>			_fxaaRenderPass;
		std::unique_ptr<DeferredCompositionPass>	_deferredCompositionPass;
		std::unique_ptr<DepthFogPass>			_depthFogPass;
		std::unique_ptr<SSAORenderPass>				_ssaoRenderPass;
		std::unique_ptr<ScreenSpaceOutlinePass>	_outlineRenderPass;
		std::unique_ptr<SkySphereRenderPass>		_skySphereRenderPass;
		std::unique_ptr<StaticMeshRenderPass>		_staticMeshRenderPass;
		std::unique_ptr<ShadowCasterPass>			_shadowCasterPass;
		std::unique_ptr<SpriteRenderPass>			_spriteRenderPass;
		std::unique_ptr<SkeletalMeshRenderPass>		_skeletalMeshRenderPass;
		std::unique_ptr<UIRenderPass>				_uiRenderPass;
		GBuffer										_gBuffer;
		PingPongRenderTarget						_sceneColor;
		std::unique_ptr<Texture>					_sceneDepth;
		ShadowSettings								_shadowSettings{};
		CascadedShadowMap							_cascadedShadowMap;
		std::shared_ptr<Texture>					_spotLightCookieTexture;
		std::vector<LightRenderItem>				_lightRenderItems;
		uint64										_renderSubmissionOrder = 0;

#if GM_ENABLE_DEBUG_TOOLS
		std::unique_ptr<RenderTargetDebugPass> _renderTargetDebugPass;
		bool _isBoundingVolumeDebugDrawEnabled = false;
		bool _isFrustumCullingEnabled = true;
		bool _isStaticMeshInstancingEnabled = true;
		bool _isScreenSpaceOutlineEnabled = true;
		bool _isFXAAEnabled = true;
		bool _showCascadeDebugColors = false;
		RenderTargetDebugView _renderTargetDebugView = RenderTargetDebugView::OriginalScene;
#endif
	};
}
