#pragma once

#include <DXUT.h>
using namespace DirectX;

struct VERTEX
{
	VERTEX();
	VERTEX(XMFLOAT3 pos, XMFLOAT4 col) { position = pos; color = col; }
	VERTEX(float x, float y, float z, XMFLOAT4  Color)
	{
		XMFLOAT3 pos = XMFLOAT3(x, y, z);
		position = pos;
		color = Color;
	}

	XMFLOAT3 position;
	XMFLOAT4  color;
};

struct CBChangesEveryFrame
{
	XMFLOAT4X4 mWorldViewProj;
	XMFLOAT4X4 mWorld;
};