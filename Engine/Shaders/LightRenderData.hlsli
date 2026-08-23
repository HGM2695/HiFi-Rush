static const uint MaxDirectionalLightCount = 4;
static const uint MaxPointLightCount = 32;
static const uint MaxSpotLightCount = 16;
static const uint SpotConeFalloffSmooth = 0;
static const uint SpotConeFalloffToonSteps = 1;

struct DirectionalLightRenderData
{
    float3  direction;
    float   padding;
    float3  color;
    float   intensity;
};

struct PointLightRenderData
{
    float3  position;
    float   range;
    float3  color;
    float   intensity;
};

struct SpotLightRenderData
{
    float3  position;
    float   range;
    float3  direction;
    float   innerConeCosine;
    float3  color;
    float   intensity;
    float   outerConeCosine;
    uint    coneFalloff;
	uint    cookieEnabled;
	uint    padding;
    float3  right;
    float   rightPadding;
};

cbuffer SceneLightConstants : register(b1)
{
    float3                      ambientColor;
    float                       ambientIntensity;
    uint                        directionalLightCount;
    uint                        pointLightCount;
    uint                        spotLightCount;
	int                         shadowDirectionalLightIndex;
    DirectionalLightRenderData  directionalLights[MaxDirectionalLightCount];
    PointLightRenderData        pointLights[MaxPointLightCount];
    SpotLightRenderData         spotLights[MaxSpotLightCount];
};
