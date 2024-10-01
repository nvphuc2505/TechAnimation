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

void Utils::GetLocalTransform(const XMFLOAT4X4& matrix, XMFLOAT3& position, XMFLOAT3& rotation, XMFLOAT3& scale)
{
	XMMATRIX mat = XMLoadFloat4x4(&matrix);

	position = { XMVectorGetX(mat.r[3]),
		XMVectorGetY(mat.r[3]),
		XMVectorGetZ(mat.r[3]) };

	scale.x = XMVectorGetX(XMVector3Length(mat.r[0]));
	scale.y = XMVectorGetX(XMVector3Length(mat.r[1]));
	scale.z = XMVectorGetX(XMVector3Length(mat.r[2]));

	mat.r[0] = XMVector3Normalize(mat.r[0]);
	mat.r[1] = XMVector3Normalize(mat.r[1]);
	mat.r[2] = XMVector3Normalize(mat.r[2]);

	rotation.y = atan2f(mat.r[2].m128_f32[0], mat.r[2].m128_f32[2]);
	rotation.x = asinf(-mat.r[2].m128_f32[1]);
	rotation.z = atan2f(mat.r[1].m128_f32[1], mat.r[0].m128_f32[1]);
}