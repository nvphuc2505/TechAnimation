#pragma once

#include <DXUT.h>
#include <assimp/postprocess.h>
using namespace DirectX;

namespace Utils
{
	XMFLOAT4X4 AssimpToXMFLOAT4X4(const aiMatrix4x4& aiMat);
	void GetLocalTransform(const XMFLOAT4X4& matrix, XMFLOAT3& position, XMFLOAT3& rotation, XMFLOAT3& scale);
}