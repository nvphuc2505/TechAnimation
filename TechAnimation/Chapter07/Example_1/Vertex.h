#pragma once

#include <DXUT.h>
using namespace DirectX;



namespace SimpleVertex
{
	struct Mesh
	{
		XMFLOAT3 Pos;
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