#pragma once

#include <DXUT.h>

class RenderState
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	// Rasterizer states
	static ID3D11RasterizerState* mWireframe;
};