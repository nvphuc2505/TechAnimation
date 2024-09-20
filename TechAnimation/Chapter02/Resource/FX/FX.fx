#include "LightingHelper.fx"

cbuffer cbChangesEveryFrame : register(b0)
{
    matrix WorldViewProj;
    matrix World;
};



struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
};



PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    float4 worldPosition = float4(input.Pos, 1.0f);
    output.Pos = mul(worldPosition, WorldViewProj);
    
    output.Normal = mul(input.Normal, (float3x3) World);

    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    float3 normalizedNormal = normalize(input.Normal);
    float4 color = float4((normalizedNormal + 1.0f) * 0.5f, 1.0f); 
    return color;
}