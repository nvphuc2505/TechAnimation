#pragma once

#include <DXUT.h>
#include <assimp/postprocess.h>
using namespace DirectX;

namespace Utils
{
	XMFLOAT4X4 AssimpToXMFLOAT4X4(const aiMatrix4x4& aiMat);
	XMFLOAT3 AssimpToXMFLOAT3(const aiVector3D& aiVec);
	XMFLOAT4 AssimpToXMFLOAT4(const aiQuaternion& aiQuat);
}