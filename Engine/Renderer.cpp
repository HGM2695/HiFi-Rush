#include "Renderer.h"
#include "BloomRenderPass.h"
#include "DeferredCompositionPass.h"
#include "DepthFogPass.h"
#include "FullscreenRenderPass.h"
#include "FXAARenderPass.h"
#include "GraphicsUtils.h"
#include "IGraphicsCommandContext.h"
#include "IGraphicsResourceFactory.h"
#include "RenderTargetDebugPass.h"
#include "ScreenSpaceOutlinePass.h"
#include "SkySphereRenderPass.h"
#include "ShadowCasterPass.h"
#include "SSAORenderPass.h"
#include "SpriteRenderPass.h"
#include "StaticMeshRenderPass.h"
#include "SkeletalMeshRenderPass.h"
#include "Texture.h"
#include "ToneMappingPass.h"
#include "ToneMappingTypes.h"
#include "UIRenderPass.h"

#include <algorithm>
#include <cmath>
#include <vector>

#if GM_ENABLE_DEBUG_TOOLS
#include "IDebugRenderer.h"
#endif

namespace gm
{
	namespace
	{
		Vector3 GetLinearLightColor(const LightRenderItem& light)
		{
			const Color color = ConvertSRGBToLinear(light.color);
			return Vector3{ color.x, color.y, color.z };
		}
	}

	Renderer::Renderer(Resources& resources, IGraphicsCommandContext& commandContext, IGraphicsResourceFactory& resourceFactory)
		: _commandContext(commandContext)
		, _resourceFactory(resourceFactory)
		, _fullscreenRenderPass(std::make_unique<FullscreenRenderPass>(resources, commandContext))
		, _bloomRenderPass(std::make_unique<BloomRenderPass>(resources, commandContext, resourceFactory, *_fullscreenRenderPass))
		, _toneMappingPass(std::make_unique<ToneMappingPass>(resources, commandContext, resourceFactory, *_fullscreenRenderPass))
		, _fxaaRenderPass(std::make_unique<FXAARenderPass>(resources, commandContext, resourceFactory, *_fullscreenRenderPass))
		, _deferredCompositionPass(std::make_unique<DeferredCompositionPass>(resources, commandContext, resourceFactory, *_fullscreenRenderPass))
		, _depthFogPass(std::make_unique<DepthFogPass>(resources, commandContext, resourceFactory, *_fullscreenRenderPass))
		, _ssaoRenderPass(std::make_unique<SSAORenderPass>(resources, commandContext, resourceFactory, *_fullscreenRenderPass))
		, _outlineRenderPass(std::make_unique<ScreenSpaceOutlinePass>(resources, commandContext, resourceFactory, *_fullscreenRenderPass))
		, _skySphereRenderPass(std::make_unique<SkySphereRenderPass>(resources, commandContext, resourceFactory))
		, _spriteRenderPass(std::make_unique<SpriteRenderPass>(resources, commandContext, resourceFactory))
		, _staticMeshRenderPass(std::make_unique<StaticMeshRenderPass>(resources, commandContext, resourceFactory))
		, _shadowCasterPass(std::make_unique<ShadowCasterPass>(resources, commandContext, resourceFactory))
		, _skeletalMeshRenderPass(std::make_unique<SkeletalMeshRenderPass>(resources, commandContext, resourceFactory))
		, _uiRenderPass(std::make_unique<UIRenderPass>(resources, commandContext, resourceFactory))
#if GM_ENABLE_DEBUG_TOOLS
		, _renderTargetDebugPass(std::make_unique<RenderTargetDebugPass>(resources, commandContext, resourceFactory, *_fullscreenRenderPass))
#endif
	{
	}

	Renderer::~Renderer() = default;

	bool Renderer::Initialize(uint32 width, uint32 height)
	{
		GM_ASSERT_RETURN_VAL(_fullscreenRenderPass->Initialize(), false, "FullscreenRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_bloomRenderPass->Initialize(width, height), false, "BloomRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_toneMappingPass->Initialize(), false, "ToneMappingPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_fxaaRenderPass->Initialize(), false, "FXAARenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_deferredCompositionPass->Initialize(), false, "DeferredCompositionPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_depthFogPass->Initialize(), false, "DepthFogPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_ssaoRenderPass->Initialize(width, height), false, "SSAORenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_outlineRenderPass->Initialize(width, height), false, "ScreenSpaceOutlinePass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_skySphereRenderPass->Initialize(), false, "SkySphereRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_spriteRenderPass->Initialize(), false, "SpriteRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_staticMeshRenderPass->Initialize(), false, "StaticMeshRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_shadowCasterPass->Initialize(), false, "ShadowCasterPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_skeletalMeshRenderPass->Initialize(), false, "SkeletalMeshRenderPass 초기화에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_uiRenderPass->Initialize(), false, "UIRenderPass 초기화에 실패했습니다.");
#if GM_ENABLE_DEBUG_TOOLS
		GM_ASSERT_RETURN_VAL(_renderTargetDebugPass->Initialize(), false, "RenderTargetDebugPass 초기화에 실패했습니다.");
#endif
		GM_ASSERT_RETURN_VAL(CreateSceneRenderTargets(width, height), false, "Scene Render Target 생성에 실패했습니다.");
		GM_ASSERT_RETURN_VAL(_cascadedShadowMap.Initialize(_resourceFactory, _shadowSettings), false, "Cascaded Shadow Map 초기화에 실패했습니다.");

		return true;
	}

	void Renderer::SubmitLight(const LightRenderItem& item)
	{
		_lightRenderItems.push_back(item);
	}

	void Renderer::SubmitSprite(const SpriteRenderItem& item)
	{
		SpriteRenderItem submittedItem = item;
		submittedItem.submissionOrder = _renderSubmissionOrder++;
		_spriteRenderPass->Submit(submittedItem);
	}

	void Renderer::SubmitStaticMesh(const StaticMeshRenderItem& item)
	{
		StaticMeshRenderItem submittedItem = item;
		submittedItem.submissionOrder = _renderSubmissionOrder++;
		_staticMeshRenderPass->Submit(submittedItem);
		if (submittedItem.castsShadow)
			_shadowCasterPass->Submit(submittedItem);
	}

	void Renderer::SubmitSkeletalMesh(const SkeletalMeshRenderItem& item)
	{
		SkeletalMeshRenderItem submittedItem = item;
		submittedItem.submissionOrder = _renderSubmissionOrder++;
		_skeletalMeshRenderPass->Submit(submittedItem);
		if (submittedItem.castsShadow)
			_shadowCasterPass->Submit(submittedItem);
	}

	void Renderer::SubmitUI(const UIRenderItem& item)
	{
		_uiRenderPass->Submit(item);
	}

	void Renderer::SetSkySphere(const std::shared_ptr<StaticMesh>& staticMesh, const std::shared_ptr<Texture>& texture)
	{
		_skySphereRenderPass->SetSkySphere(staticMesh, texture);
	}

	void Renderer::ClearSkySphere()
	{
		_skySphereRenderPass->ClearSkySphere();
	}

	void Renderer::SetBloomSettings(const BloomSettings& settings)
	{
		_bloomRenderPass->SetSettings(settings);
	}

	const BloomSettings& Renderer::GetBloomSettings() const
	{
		return _bloomRenderPass->GetSettings();
	}

	void Renderer::SetSSAOSettings(const SSAOSettings& settings)
	{
		_ssaoRenderPass->SetSettings(settings);
	}

	const SSAOSettings& Renderer::GetSSAOSettings() const
	{
		return _ssaoRenderPass->GetSettings();
	}

	void Renderer::SetScreenSpaceOutlineSettings(const ScreenSpaceOutlineSettings& settings)
	{
		_outlineRenderPass->SetSettings(settings);
	}

	const ScreenSpaceOutlineSettings& Renderer::GetScreenSpaceOutlineSettings() const
	{
		return _outlineRenderPass->GetSettings();
	}

#if GM_ENABLE_DEBUG_TOOLS
	void Renderer::SelectNextRenderTargetDebugView()
	{
		const uint32 nextView = (static_cast<uint32>(_renderTargetDebugView) + 1) % static_cast<uint32>(RenderTargetDebugView::Count);
		_renderTargetDebugView = static_cast<RenderTargetDebugView>(nextView);
	}

	bool Renderer::SetShadowResolution(uint32 resolution)
	{
		if (_shadowSettings.mapResolution == resolution)
			return true;
		ShadowSettings settings = _shadowSettings;
		settings.mapResolution = resolution;
		if (_cascadedShadowMap.Initialize(_resourceFactory, settings) == false)
			return false;
		_shadowSettings = settings;
		return true;
	}

	void Renderer::SetShadowCascadeCount(uint32 cascadeCount)
	{
		_shadowSettings.cascadeCount = std::clamp(cascadeCount, 1u, MaxShadowCascadeCount);
		_cascadedShadowMap.UpdateSettings(_shadowSettings);
	}

	void Renderer::SetShadowPCFRadius(uint32 pcfRadius)
	{
		_shadowSettings.pcfRadius = pcfRadius;
		_cascadedShadowMap.UpdateSettings(_shadowSettings);
	}

	void Renderer::SetShadowStrength(float strength)
	{
		_shadowSettings.shadowStrength = std::clamp(strength, 0.f, 1.f);
		_cascadedShadowMap.UpdateSettings(_shadowSettings);
	}

	void Renderer::SetShadowCascadeSplitLambda(float lambda)
	{
		_shadowSettings.cascadeSplitLambda = std::clamp(lambda, 0.f, 1.f);
		_cascadedShadowMap.UpdateSettings(_shadowSettings);
	}

	void Renderer::SetShadowMaxDistance(float distance)
	{
		_shadowSettings.maxDistance = std::clamp(distance, 5.f, 500.f);
		_cascadedShadowMap.UpdateSettings(_shadowSettings);
	}

	void Renderer::SetShadowDepthBias(float bias)
	{
		_shadowSettings.depthBias = std::clamp(bias, 0.f, 0.1f);
		_cascadedShadowMap.UpdateSettings(_shadowSettings);
	}

	void Renderer::SetShadowNormalBias(float bias)
	{
		_shadowSettings.normalBias = std::clamp(bias, 0.f, 1.f);
		_cascadedShadowMap.UpdateSettings(_shadowSettings);
	}

	void Renderer::SetShadowColor(const Color& color)
	{
		_shadowSettings.shadowColor = color;
		_cascadedShadowMap.UpdateSettings(_shadowSettings);
	}

	CullingDebugStats Renderer::GetCullingDebugStats() const
	{
		CullingDebugStats stats{};
		stats.staticMesh.submittedCount = _staticMeshRenderPass->GetLastSubmittedItemCount();
		stats.staticMesh.visibleCount = _staticMeshRenderPass->GetLastVisibleItemCount();
		stats.staticMesh.culledCount = _staticMeshRenderPass->GetLastCulledItemCount();
		stats.skeletalMesh.submittedCount = _skeletalMeshRenderPass->GetLastSubmittedItemCount();
		stats.skeletalMesh.visibleCount = _skeletalMeshRenderPass->GetLastVisibleItemCount();
		stats.skeletalMesh.culledCount = _skeletalMeshRenderPass->GetLastCulledItemCount();
		return stats;
	}

	StaticMeshInstancingDebugStats Renderer::GetStaticMeshInstancingDebugStats() const
	{
		StaticMeshInstancingDebugStats stats{};
		stats.renderBatchCount = _staticMeshRenderPass->GetLastRenderBatchCount();
		stats.normalDrawCallCount = _staticMeshRenderPass->GetLastNormalDrawCallCount();
		stats.instancedDrawCallCount = _staticMeshRenderPass->GetLastInstancedDrawCallCount();
		stats.instancedInstanceCount = _staticMeshRenderPass->GetLastInstancedInstanceCount();
		return stats;
	}

	void Renderer::DebugDraw(IDebugRenderer& debugRenderer) const
	{
		if (_isBoundingVolumeDebugDrawEnabled == false)
			return;

		_staticMeshRenderPass->DebugDraw(debugRenderer);
		_skeletalMeshRenderPass->DebugDraw(debugRenderer);
	}
#endif

	void Renderer::Render(const SceneAmbientSettings& ambientSettings, const DepthFogSettings& depthFogSettings, const ToneMappingSettings& toneMappingSettings, const CameraViewInfo& viewInfo, uint32 width, uint32 height, const Color& clearColor)
	{
		_sceneColor.Reset();
		_commandContext.ClearRenderTarget(_sceneColor.GetDestination(), Colors::Transparent);
		_gBuffer.Clear(_commandContext, clearColor);
		_commandContext.ClearDepthStencil(*_sceneDepth);
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(width), static_cast<float>(height) });

		const BoundingFrustum worldFrustum = CreateWorldFrustum(viewInfo.view, viewInfo.projection);
		const BoundingFrustum* worldFrustumPtr = &worldFrustum;

#if GM_ENABLE_DEBUG_TOOLS
		if (_isFrustumCullingEnabled == false)
			worldFrustumPtr = nullptr;
#endif

		bool isStaticMeshInstancingEnabled = true;
#if GM_ENABLE_DEBUG_TOOLS
		isStaticMeshInstancingEnabled = _isStaticMeshInstancingEnabled;
#endif
		const LightRenderData lightRenderData = CollectLightRenderData(ambientSettings, viewInfo, worldFrustum);
		if (lightRenderData.shadowDirectionalLightIndex >= 0)
		{
			const Vector3 shadowLightDirection = lightRenderData.directionalLights[lightRenderData.shadowDirectionalLightIndex].direction;
			_cascadedShadowMap.ClearCascades();
			_cascadedShadowMap.UpdateCascades(viewInfo, shadowLightDirection);
			for (uint32 cascadeIndex = 0; cascadeIndex < _shadowSettings.cascadeCount; ++cascadeIndex)
			{
				const ShadowCascade& cascade = _cascadedShadowMap.GetCascade(cascadeIndex);
				_shadowCasterPass->RenderCascade(_cascadedShadowMap, cascadeIndex, cascade.viewProjection, &cascade.casterBounds, isStaticMeshInstancingEnabled);
			}
		}
		else
		{
			_cascadedShadowMap.ClearCascades();
		}
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(width), static_cast<float>(height) });
		_staticMeshRenderPass->Prepare(viewInfo, worldFrustumPtr);
		_skeletalMeshRenderPass->Prepare(viewInfo, worldFrustumPtr);
		_spriteRenderPass->Prepare(viewInfo);
		_commandContext.BindRenderTargets(_gBuffer.GetRenderTargets(), _sceneDepth.get());
		_skySphereRenderPass->Render(viewInfo);
		_staticMeshRenderPass->RenderOpaqueAndMasked(isStaticMeshInstancingEnabled);
		_skeletalMeshRenderPass->RenderOpaqueAndMasked();
		_ssaoRenderPass->Render(*_sceneDepth, _gBuffer.GetTarget(GBufferTarget::WorldNormal), viewInfo);
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(width), static_cast<float>(height) });

		bool showCascadeDebugColors = false;
#if GM_ENABLE_DEBUG_TOOLS
		showCascadeDebugColors = _showCascadeDebugColors;
#endif
		_deferredCompositionPass->Render(_gBuffer, *_sceneDepth, _ssaoRenderPass->GetAmbientOcclusionTexture(), _sceneColor.GetSource(), viewInfo, lightRenderData, _cascadedShadowMap.GetRenderData(), _cascadedShadowMap.GetTexture(), _spotLightCookieTexture.get(), showCascadeDebugColors);
		bool isScreenSpaceOutlineEnabled = true;
		bool showScreenSpaceOutlineDebug = false;
#if GM_ENABLE_DEBUG_TOOLS
		isScreenSpaceOutlineEnabled = _isScreenSpaceOutlineEnabled;
		showScreenSpaceOutlineDebug = _renderTargetDebugView == RenderTargetDebugView::ScreenSpaceOutline;
#endif
		if (isScreenSpaceOutlineEnabled)
		{
			_outlineRenderPass->Render(_sceneColor.GetSource(), *_sceneDepth, _gBuffer.GetTarget(GBufferTarget::WorldNormal), _gBuffer.GetTarget(GBufferTarget::MaterialData), _sceneColor.GetDestination(), viewInfo, width, height);
			_sceneColor.Swap();
		}
		if (showScreenSpaceOutlineDebug)
			_outlineRenderPass->RenderDebug(_sceneColor.GetSource(), *_sceneDepth, _gBuffer.GetTarget(GBufferTarget::WorldNormal), _gBuffer.GetTarget(GBufferTarget::MaterialData), viewInfo, width, height);
		if (depthFogSettings.enabled)
		{
			_depthFogPass->Render(_sceneColor.GetSource(), *_sceneDepth, _sceneColor.GetDestination(), viewInfo, depthFogSettings);
			_sceneColor.Swap();
		}
		_lightRenderItems.clear();
		Texture& sceneColor = _sceneColor.GetSource();
		_commandContext.BindRenderTarget(&sceneColor, _sceneDepth.get());
		_depthFogPass->BindForwardConstants(depthFogSettings);

		std::vector<TransparentRenderEntry> transparentEntries;
		_staticMeshRenderPass->AppendTransparentRenderEntries(transparentEntries);
		_skeletalMeshRenderPass->AppendTransparentRenderEntries(transparentEntries);
		_spriteRenderPass->AppendTransparentRenderEntries(transparentEntries);
		std::stable_sort(transparentEntries.begin(), transparentEntries.end(), [](const TransparentRenderEntry& lhs, const TransparentRenderEntry& rhs)
		{
			if (lhs.cameraDepth != rhs.cameraDepth)
				return lhs.cameraDepth > rhs.cameraDepth;
			return lhs.submissionOrder < rhs.submissionOrder;
		});

		for (const TransparentRenderEntry& entry : transparentEntries)
		{
			if (entry.source == TransparentRenderSource::Static)
				_staticMeshRenderPass->RenderTransparent(entry.itemIndex);
			else if (entry.source == TransparentRenderSource::Skeletal)
				_skeletalMeshRenderPass->RenderTransparent(entry.itemIndex);
			else
				_spriteRenderPass->Render(entry.itemIndex);
		}

		_bloomRenderPass->Render(_sceneColor.GetSource(), _sceneColor.GetDestination());
		_sceneColor.Swap();

		_spriteRenderPass->Clear();
		_staticMeshRenderPass->Clear();
		_skeletalMeshRenderPass->Clear();
		_shadowCasterPass->Clear();
		_renderSubmissionOrder = 0;

		bool renderOriginalScene = true;
#if GM_ENABLE_DEBUG_TOOLS
		renderOriginalScene = _renderTargetDebugView == RenderTargetDebugView::OriginalScene;
#endif
		if (renderOriginalScene)
		{
			_commandContext.BindRenderTarget(&_sceneColor.GetDestination(), nullptr);
			_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(width), static_cast<float>(height) });
			_toneMappingPass->Render(_sceneColor.GetSource(), toneMappingSettings);
			_sceneColor.Swap();
		}

		_commandContext.BindBackBuffer();
		_commandContext.ClearBackBuffer(clearColor);
		_commandContext.SetViewport(Viewport{ 0.f, 0.f, static_cast<float>(width), static_cast<float>(height) });

#if GM_ENABLE_DEBUG_TOOLS
		if (renderOriginalScene)
		{
			if (_isFXAAEnabled)
				_fxaaRenderPass->Render(_sceneColor.GetSource());
			else
				_fullscreenRenderPass->Copy(_sceneColor.GetSource());
		}
		else
		{
			const float debugValueScale = _renderTargetDebugView == RenderTargetDebugView::BloomContribution ? _bloomRenderPass->GetSettings().intensity : 1.f;
			_renderTargetDebugPass->Render(GetRenderTargetDebugTexture(), _renderTargetDebugView, viewInfo, debugValueScale);
		}
#else
		_fxaaRenderPass->Render(_sceneColor.GetSource());
#endif
		_uiRenderPass->Render(width, height);
	}

	LightRenderData Renderer::CollectLightRenderData(const SceneAmbientSettings& ambientSettings, const CameraViewInfo& viewInfo, const BoundingFrustum& worldFrustum) const
	{
		LightRenderData renderData{};
		const Color ambientColor = ConvertSRGBToLinear(ambientSettings.ambientColor);
		renderData.ambientColor = Vector3{ ambientColor.x, ambientColor.y, ambientColor.z };
		renderData.ambientIntensity = ambientSettings.ambientIntensity;

		std::vector<const LightRenderItem*> visiblePointLights;
		std::vector<const LightRenderItem*> visibleSpotLights;
		for (const LightRenderItem& light : _lightRenderItems)
		{
			switch (light.lightType)
			{
			case LightType::Directional:
			{
				if (renderData.directionalLightCount >= MaxDirectionalLightCount)
					break;

				const uint32 directionalLightIndex = renderData.directionalLightCount++;
				DirectionalLightRenderData& data = renderData.directionalLights[directionalLightIndex];
				data.direction = light.direction;
				data.color = GetLinearLightColor(light);
				data.intensity = light.intensity;
				if (light.castsShadow && renderData.shadowDirectionalLightIndex < 0)
					renderData.shadowDirectionalLightIndex = static_cast<int32>(directionalLightIndex);
				break;
			}

			case LightType::Point:
			{
				if (worldFrustum.Intersects(BoundingSphere(light.position, light.range)))
					visiblePointLights.push_back(&light);
				break;
			}

			case LightType::Spot:
			{
				if (worldFrustum.Intersects(BoundingSphere(light.position, light.range)))
					visibleSpotLights.push_back(&light);
				break;
			}

			default:
				break;
			}
		}

		auto sortByCameraDistance = [&viewInfo](const LightRenderItem* lhs, const LightRenderItem* rhs)
		{
			return (lhs->position - viewInfo.position).LengthSquared() < (rhs->position - viewInfo.position).LengthSquared();
		};
		std::stable_sort(visiblePointLights.begin(), visiblePointLights.end(), sortByCameraDistance);
		std::stable_sort(visibleSpotLights.begin(), visibleSpotLights.end(), sortByCameraDistance);

		renderData.pointLightCount = std::min(static_cast<uint32>(visiblePointLights.size()), MaxPointLightCount);
		for (uint32 lightIndex = 0; lightIndex < renderData.pointLightCount; ++lightIndex)
		{
			const LightRenderItem& light = *visiblePointLights[lightIndex];
			PointLightRenderData& data = renderData.pointLights[lightIndex];
			data.position = light.position;
			data.range = light.range;
			data.color = GetLinearLightColor(light);
			data.intensity = light.intensity;
		}

		renderData.spotLightCount = std::min(static_cast<uint32>(visibleSpotLights.size()), MaxSpotLightCount);
		for (uint32 lightIndex = 0; lightIndex < renderData.spotLightCount; ++lightIndex)
		{
			const LightRenderItem& light = *visibleSpotLights[lightIndex];
			SpotLightRenderData& data = renderData.spotLights[lightIndex];
			data.position = light.position;
			data.range = light.range;
			data.direction = light.direction;
			data.innerConeCosine = std::cos(light.innerConeRadian);
			data.outerConeCosine = std::cos(light.outerConeRadian);
			data.coneFalloff = static_cast<uint32>(light.spotConeFalloff);
			data.cookieEnabled = light.useSpotCookie ? 1u : 0u;
			data.right = light.right;
			data.color = GetLinearLightColor(light);
			data.intensity = light.intensity;
		}

		return renderData;
	}

#if GM_ENABLE_DEBUG_TOOLS
	const Texture& Renderer::GetRenderTargetDebugTexture() const
	{
		switch (_renderTargetDebugView)
		{
		case RenderTargetDebugView::BaseColor:
			return _gBuffer.GetTarget(GBufferTarget::BaseColor);
		case RenderTargetDebugView::WorldNormal:
			return _gBuffer.GetTarget(GBufferTarget::WorldNormal);
		case RenderTargetDebugView::AmbientOcclusion:
			return _gBuffer.GetTarget(GBufferTarget::MaterialData);
		case RenderTargetDebugView::ScreenSpaceAmbientOcclusion:
			return _ssaoRenderPass->GetAmbientOcclusionTexture();
		case RenderTargetDebugView::ScreenSpaceOutline:
			return _outlineRenderPass->GetDebugTexture();
		case RenderTargetDebugView::MaterialFlags:
			return _gBuffer.GetTarget(GBufferTarget::MaterialData);
		case RenderTargetDebugView::Emissive:
			return _gBuffer.GetTarget(GBufferTarget::Emissive);
		case RenderTargetDebugView::SceneDepth:
			return *_sceneDepth;
		case RenderTargetDebugView::BloomContribution:
			return _bloomRenderPass->GetBloomTexture();
		case RenderTargetDebugView::SceneColorA:
			return _sceneColor.GetTargetA();
		case RenderTargetDebugView::SceneColorB:
			return _sceneColor.GetTargetB();
		default:
			return _sceneColor.GetSource();
		}
	}
#endif

	bool Renderer::CreateSceneRenderTargets(uint32 width, uint32 height)
	{
		GM_ASSERT_RETURN_VAL(_gBuffer.Initialize(_resourceFactory, width, height), false, "G-Buffer 생성에 실패했습니다.");

		TextureDesc colorDesc{};
		colorDesc.width = width;
		colorDesc.height = height;
		colorDesc.format = TextureFormat::RGBA16Float;
		colorDesc.bindUsage = TextureBindUsage::RenderTarget | TextureBindUsage::ShaderResource;
		GM_ASSERT_RETURN_VAL(_sceneColor.Initialize(_resourceFactory, colorDesc), false, "HDR SceneColor A/B 생성에 실패했습니다.");

		TextureDesc depthDesc{};
		depthDesc.width = width;
		depthDesc.height = height;
		depthDesc.format = TextureFormat::Depth24Stencil8;
		depthDesc.bindUsage = TextureBindUsage::DepthStencil | TextureBindUsage::ShaderResource;
		_sceneDepth = _resourceFactory.CreateTexture(depthDesc);
		GM_ASSERT_RETURN_VAL(_sceneDepth, false, "SceneDepth 생성에 실패했습니다.");
		return true;
	}

	void Renderer::Clear()
	{
		_spriteRenderPass->Clear();
		_staticMeshRenderPass->Clear();
		_skeletalMeshRenderPass->Clear();
		_shadowCasterPass->Clear();
		_uiRenderPass->Clear();
		_lightRenderItems.clear();
		_renderSubmissionOrder = 0;
	}
}
