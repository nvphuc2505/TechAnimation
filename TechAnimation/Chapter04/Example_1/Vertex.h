#pragma once

#include <DXUT.h>
using namespace DirectX;



namespace SkinnedVertex
{
	struct Mesh
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		BYTE BoneID[4];
		XMFLOAT4 BoneWeight;
	};
}

struct CBChangesEveryFrame
{
	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mWorldInvTranspose;
	XMFLOAT4X4 mWorldViewProj;
	XMFLOAT4X4 TexcoordTransform;
};

struct CBSkinned
{
	XMFLOAT4X4 gBoneTransforms[50];
};