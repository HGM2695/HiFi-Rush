cbuffer ObjectConstants : register(b0)
{
    row_major matrix world;
};

cbuffer CameraConstants : register(b1)
{
    row_major matrix view;
    row_major matrix projection;
}

struct VSInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    
    float4 worldPosition = mul(float4(input.position, 1.f), world);
    float4 viewPosition = mul(worldPosition, view);
    output.position = mul(viewPosition, projection);
    output.texcoord = input.texcoord;
    return output;
}
