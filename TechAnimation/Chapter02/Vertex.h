#pragma once

#include <DXUT.h>
using namespace DirectX;

namespace Vertex
{
	struct Basic32
	{
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};
}