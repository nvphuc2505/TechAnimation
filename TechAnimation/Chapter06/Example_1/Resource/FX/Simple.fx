cbuffer cbChangesEveryFrame 
{
    float4x4 World;
    float4x4 WorldInvTranspose;
    float4x4 WorldViewProj;
    float4x4 TexcoordTransform;
};

struct VertexIn
{
    float3 Position : POSITION;
};

struct VertexOut
{
    float4 PositionH : SV_POSITION;
};



VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.PositionH = mul(float4(vin.Position, 1.0f), WorldViewProj);

    return vout;
}

float4 PS(VertexOut vout) : SV_Target
{
    return float4(0.0f, 1.0f, 0.8f, 1.0f);
}