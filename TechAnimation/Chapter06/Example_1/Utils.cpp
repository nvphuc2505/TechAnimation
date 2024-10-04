#include "Utils.h"

XMFLOAT4X4 Utils::AssimpToXMFLOAT4X4(const aiMatrix4x4& aiMat)
{
	return XMFLOAT4X4
	(
		aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
		aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
		aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
		aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
	);
}

XMFLOAT3 Utils::AssimpToXMFLOAT3(const aiVector3D& aiVec)
{
	return XMFLOAT3
	(
		aiVec.x,
		aiVec.y,
		aiVec.z
	);
}

XMFLOAT4 Utils::AssimpToXMFLOAT4(const aiQuaternion& aiQuat)
{
	return XMFLOAT4
	(
		aiQuat.x,
		aiQuat.y,
		aiQuat.z,
		aiQuat.w
	);
}

