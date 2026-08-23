#include "DeferredCommon.hlsli"
#include "GBufferMaterialData.hlsli"
#include "LightRenderData.hlsli"
#include "CascadedShadowRenderData.hlsli"

Texture2D baseColorTexture : register(t0);
Texture2D worldNormalTexture : register(t1);
Texture2D materialDataTexture : register(t2);
Texture2D emissiveTexture : register(t3);
Texture2D sceneDepthTexture : register(t4);
Texture2D spotLightCookieTexture : register(t5);
Texture2DArray cascadedShadowTexture : register(t6);
Texture2D screenSpaceAmbientOcclusionTexture : register(t7);
SamplerState gBufferSampler : register(s0);
SamplerState spotLightCookieSampler : register(s1);
SamplerState shadowSampler : register(s2);
SamplerState ambientOcclusionSampler : register(s3);

cbuffer DeferredCompositionConstants : register(b0)
{
	row_major matrix inverseViewProjection;
	row_major matrix viewMatrix;
	uint showCascadeDebugColors;
	float3 deferredCompositionPadding;
};

static const float ToonDiffuseStepCount = 4.f;
static const float ToonSpotStepCount = 4.f;

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float CalcDistanceAttenuation(float distanceSquared, float lightRange)
{
    const float lightRangeSquared = lightRange * lightRange;
    const float normalizedAttenuation = saturate(1.f - distanceSquared / lightRangeSquared);
    return normalizedAttenuation * normalizedAttenuation;
}

float QuantizeLambert(float lambertFactor)
{
    return round(saturate(lambertFactor) * ToonDiffuseStepCount) / ToonDiffuseStepCount;
}

float CalcToonSpotAttenuation(float normalizedAttenuation)
{
	return round(saturate(normalizedAttenuation) * ToonSpotStepCount) / ToonSpotStepCount;
}

float CalcSpotConeAttenuation(SpotLightRenderData light, float coneAngleCosine)
{
    float coneAttenuation = smoothstep(light.outerConeCosine, light.innerConeCosine, coneAngleCosine);
    if (light.coneFalloff == SpotConeFalloffToonSteps)
	{
		const float normalizedConePosition = (coneAngleCosine - light.outerConeCosine) / max(light.innerConeCosine - light.outerConeCosine, 0.0001f);
		coneAttenuation = CalcToonSpotAttenuation(normalizedConePosition);
	}
    return coneAttenuation;
}

float2 CalcSpotCookieUV(SpotLightRenderData light, float3 worldPosition)
{
	const float3 lightDirection = normalize(light.direction);
	const float3 lightRight = normalize(light.right);
	const float3 lightUp = normalize(cross(lightDirection, lightRight));
	const float3 lightToSurface = worldPosition - light.position;
	const float lightSpaceDepth = dot(lightToSurface, lightDirection);
	const float outerConeSine = sqrt(saturate(1.f - light.outerConeCosine * light.outerConeCosine));
	const float lightPlaneHalfExtent = lightSpaceDepth * outerConeSine / max(light.outerConeCosine, 0.0001f);
	const float2 lightPlanePosition = float2(dot(lightToSurface, lightRight), dot(lightToSurface, lightUp));
	return float2(0.5f + lightPlanePosition.x / (2.f * lightPlaneHalfExtent), 0.5f - lightPlanePosition.y / (2.f * lightPlaneHalfExtent));
}

float CalcDiffuseFactor(float3 worldNormal, float3 surfaceToLight, uint shadingModel)
{
    const float lambertFactor = saturate(dot(worldNormal, surfaceToLight));
    return shadingModel == GBufferToonLitShadingModel ? QuantizeLambert(lambertFactor) : lambertFactor;
}

float3 CalcDirectionalDiffuse(DirectionalLightRenderData light, float3 baseColor, float3 worldNormal, uint shadingModel)
{
    const float3 surfaceToLight = normalize(-light.direction);
    const float diffuseFactor = CalcDiffuseFactor(worldNormal, surfaceToLight, shadingModel);
    return baseColor * light.color * light.intensity * diffuseFactor;
}

uint FindShadowCascadeIndex(float viewSpaceDepth)
{
	uint cascadeIndex = MaxShadowCascadeCount;
	if (cascadeCount > 0 && viewSpaceDepth <= cascadeSplitDistances.x)
		cascadeIndex = 0;
	else if (cascadeCount > 1 && viewSpaceDepth <= cascadeSplitDistances.y)
		cascadeIndex = 1;
	else if (cascadeCount > 2 && viewSpaceDepth <= cascadeSplitDistances.z)
		cascadeIndex = 2;
	else if (cascadeCount > 3 && viewSpaceDepth <= cascadeSplitDistances.w)
		cascadeIndex = 3;
	return cascadeIndex;
}

float3 CalcShadowFactor(float3 worldPosition, float3 worldNormal, uint cascadeIndex, float3 lightDirection)
{
	float filteredVisibility = 1.f;
	if (cascadeIndex < MaxShadowCascadeCount)
	{
		const float3 surfaceToLight = normalize(-lightDirection);
		const float normalOffsetScale = 1.f - saturate(dot(worldNormal, surfaceToLight));
		const float3 biasedWorldPosition = worldPosition + worldNormal * shadowNormalBias * normalOffsetScale;
		const float4 lightClipSpacePosition = mul(float4(biasedWorldPosition, 1.f), cascadeViewProjections[cascadeIndex]);
		const float3 lightNormalizedDevicePosition = lightClipSpacePosition.xyz / lightClipSpacePosition.w;
		const float2 shadowTextureCoordinate = float2(lightNormalizedDevicePosition.x * 0.5f + 0.5f, 0.5f - lightNormalizedDevicePosition.y * 0.5f);
		const float receiverShadowDepth = lightNormalizedDevicePosition.z - shadowDepthBias;
		if (receiverShadowDepth > 0.f && receiverShadowDepth < 1.f && all(shadowTextureCoordinate >= 0.f) && all(shadowTextureCoordinate <= 1.f))
		{
			float visibleSampleCount = 0.f;
			uint sampleCount = 0;
			for (int offsetY = -int(shadowPCFRadius); offsetY <= int(shadowPCFRadius); ++offsetY)
			{
				for (int offsetX = -int(shadowPCFRadius); offsetX <= int(shadowPCFRadius); ++offsetX)
				{
					const float2 sampleTextureCoordinate = shadowTextureCoordinate + float2(offsetX, offsetY) * shadowMapTexelSize;
					if (any(sampleTextureCoordinate < 0.f) || any(sampleTextureCoordinate > 1.f))
						visibleSampleCount += 1.f;
					else
					{
						const float storedShadowDepth = cascadedShadowTexture.SampleLevel(shadowSampler, float3(sampleTextureCoordinate, cascadeIndex), 0.f).r;
						visibleSampleCount += receiverShadowDepth <= storedShadowDepth ? 1.f : 0.f;
					}
					++sampleCount;
				}
			}
			filteredVisibility = visibleSampleCount / max(float(sampleCount), 1.f);
		}
	}
	const float3 filteredShadowColor = lerp(shadowColor, 1.f, filteredVisibility);
	return lerp(1.f, filteredShadowColor, saturate(shadowStrength));
}

float3 CalcPointDiffuse(PointLightRenderData light, float3 baseColor, float3 worldNormal, float3 worldPosition, uint shadingModel)
{
    const float3 surfaceToLightVector = light.position - worldPosition;
    const float lightDistanceSquared = dot(surfaceToLightVector, surfaceToLightVector);
    const float lightDistance = sqrt(lightDistanceSquared);
    const float3 surfaceToLight = surfaceToLightVector / lightDistance;
    const float diffuseFactor = CalcDiffuseFactor(worldNormal, surfaceToLight, shadingModel);
    const float distanceAttenuation = CalcDistanceAttenuation(lightDistanceSquared, light.range);
    return baseColor * light.color * light.intensity * diffuseFactor * distanceAttenuation;
}

float3 CalcSpotDiffuse(SpotLightRenderData light, float3 baseColor, float3 worldNormal, float3 worldPosition, uint shadingModel)
{
    const float3 surfaceToLightVector = light.position - worldPosition;
    const float lightDistanceSquared = dot(surfaceToLightVector, surfaceToLightVector);
    const float lightDistance = sqrt(lightDistanceSquared);
    const float3 surfaceToLight = surfaceToLightVector / lightDistance;
    const float diffuseFactor = CalcDiffuseFactor(worldNormal, surfaceToLight, shadingModel);
    const float distanceAttenuation = CalcDistanceAttenuation(lightDistanceSquared, light.range);
    const float coneAngleCosine = dot(normalize(light.direction), -surfaceToLight);
    float coneAttenuation = CalcSpotConeAttenuation(light, coneAngleCosine);
	if (light.cookieEnabled && coneAttenuation > 0.f)
	{
		const float cookieMask = spotLightCookieTexture.SampleLevel(spotLightCookieSampler, CalcSpotCookieUV(light, worldPosition), 0.f).r;
		coneAttenuation = light.coneFalloff == SpotConeFalloffToonSteps ? CalcToonSpotAttenuation(cookieMask) : coneAttenuation * cookieMask;
	}
    return baseColor * light.color * light.intensity * diffuseFactor * distanceAttenuation * coneAttenuation;
}

float3 CalcLitColor(float3 baseColor, float3 worldNormal, float3 worldPosition, uint cascadeIndex, float ambientOcclusion, uint shadingModel)
{
    float3 lightingColor = baseColor * ambientColor * ambientIntensity * saturate(ambientOcclusion);

    for (uint directionalIndex = 0; directionalIndex < directionalLightCount; ++directionalIndex)
    {
        float3 shadowFactor = 1.f;
        if (int(directionalIndex) == shadowDirectionalLightIndex)
            shadowFactor = CalcShadowFactor(worldPosition, worldNormal, cascadeIndex, directionalLights[directionalIndex].direction);
        lightingColor += CalcDirectionalDiffuse(directionalLights[directionalIndex], baseColor, worldNormal, shadingModel) * shadowFactor;
    }

    for (uint pointIndex = 0; pointIndex < pointLightCount; ++pointIndex)
        lightingColor += CalcPointDiffuse(pointLights[pointIndex], baseColor, worldNormal, worldPosition, shadingModel);

    for (uint spotIndex = 0; spotIndex < spotLightCount; ++spotIndex)
        lightingColor += CalcSpotDiffuse(spotLights[spotIndex], baseColor, worldNormal, worldPosition, shadingModel);

    return lightingColor;
}

float3 GetCascadeDebugColor(uint cascadeIndex)
{
	float3 cascadeColor = float3(1.f, 1.f, 0.2f);
	if (cascadeIndex == 0)
		cascadeColor = float3(1.f, 0.2f, 0.2f);
	else if (cascadeIndex == 1)
		cascadeColor = float3(0.2f, 1.f, 0.2f);
	else if (cascadeIndex == 2)
		cascadeColor = float3(0.2f, 0.4f, 1.f);
	return cascadeColor;
}

float4 main(PSInput input) : SV_TARGET
{
    const float4 baseColor = baseColorTexture.Sample(gBufferSampler, input.texcoord);
    const float3 worldNormal = normalize(worldNormalTexture.Sample(gBufferSampler, input.texcoord).xyz);
    const float4 materialData = materialDataTexture.Sample(gBufferSampler, input.texcoord);
    const float3 emissive = emissiveTexture.Sample(gBufferSampler, input.texcoord).rgb;
	const float sceneDepth = sceneDepthTexture.Sample(gBufferSampler, input.texcoord).r;
	const float screenSpaceAmbientOcclusion = screenSpaceAmbientOcclusionTexture.Sample(ambientOcclusionSampler, input.texcoord).r;
    
    const float3 worldPosition = ScreenUVToWorldPosition(input.texcoord, sceneDepth, inverseViewProjection);
    const float viewSpaceDepth = mul(float4(worldPosition, 1.f), viewMatrix).z;
	const uint cascadeIndex = FindShadowCascadeIndex(viewSpaceDepth);
    const uint materialFlags = DecodeGBufferMaterialFlags(materialData.a);
    const uint shadingModel = materialFlags & GBufferShadingModelMask;

    float3 sceneColor = baseColor.rgb;
    if (shadingModel == GBufferLitShadingModel || shadingModel == GBufferToonLitShadingModel)
		sceneColor = CalcLitColor(baseColor.rgb, worldNormal, worldPosition, cascadeIndex, materialData.r * screenSpaceAmbientOcclusion, shadingModel);
    else if (shadingModel == GBufferUnlitShadingModel)
        sceneColor = baseColor.rgb;

	if (showCascadeDebugColors && cascadeIndex < cascadeCount)
		sceneColor = lerp(sceneColor, GetCascadeDebugColor(cascadeIndex), 0.35f);

    return float4(sceneColor + emissive, baseColor.a);
}
