cbuffer cbChangesEveryFrame
{
    matrix World;
    matrix WorldInvTranspose;
    matrix WorldViewProj;
    matrix TexcoordTransform;
};



struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexL : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
};



VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.PosW    = mul(float4(vin.PosL, 1.0f), World).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) WorldInvTranspose);
    vout.Tex     = mul(float4(vin.TexL, 0.0f, 1.0f), TexcoordTransform).xy;
    vout.PosH    = mul(float4(vin.PosL, 1.0f), WorldViewProj);
    
    return vout;
}

float4 PS(VertexOut vout) : SV_Target
{
    vout.NormalW = normalize(vout.NormalW);
    float4 color = float4((vout.NormalW + 1.0f) * 0.5f, 0.5f);
    return color;
}