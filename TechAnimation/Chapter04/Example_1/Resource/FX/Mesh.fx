cbuffer cbChangesEveryFrame : register(b0)
{
    float4x4 World;
    float4x4 WorldInvTranspose;
    float4x4 WorldViewProj;
    float4x4 TexcoordTransform;
};

cbuffer cbSkinned : register(b1)
{
    float4x4 gBoneTransforms[50];
};



struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexL : TEXCOORD;
    float4 BoneWeight : BONEWEIGHT;
    uint4 BoneID : BONEID;
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
    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = vin.BoneWeight.x;
    weights[1] = vin.BoneWeight.y;
    weights[2] = vin.BoneWeight.z;
    weights[3] = vin.BoneWeight.w;

    float4 posL = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < 4; ++i)
    {
        if (vin.BoneID[i] == -1)
            continue;
        
        float4 localPosition = mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneID[i]]);
        posL += weights[i] * localPosition;
        normalL += weights[i] * mul(vin.NormalL, (float3x3) gBoneTransforms[vin.BoneID[i]]);
    }
 
    
    VertexOut vout;

    vout.PosW = mul(posL, World).xyz;
    vout.NormalW = mul(normalL, (float3x3) WorldInvTranspose);
    vout.Tex = mul(float4(vin.TexL, 0.0f, 1.0f), TexcoordTransform).xy;
    vout.PosH = mul(posL, WorldViewProj);

    return vout;
}

float4 PS(VertexOut vout) : SV_Target
{
    float3 normalizedNormal = normalize(vout.NormalW);
    float4 color = float4((normalizedNormal + 1.0f) * 0.5f, 1.0f);
    return color;
}