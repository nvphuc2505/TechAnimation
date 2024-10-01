#pragma once

#include <DXUT.h>
using namespace DirectX;



namespace SkinnedVertex
{
	struct Bone
	{
		Bone(XMFLOAT3 Position, XMFLOAT4 Col)
		{
			Pos = Position;
			Color = Col;
		}

		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

	struct Mesh
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};
}



struct CBChangesEveryFrame
{
	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mWorldInvTranspose;
	XMFLOAT4X4 mWorldViewProj;
	XMFLOAT4X4 TexcoordTransform;
};