#include "OBB.h"





OBB::OBB()
{
    mSize = XMFLOAT3(1.0f, 1.0f, 1.0f);
    mPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
    
    XMVECTOR identity = XMQuaternionIdentity();
    XMStoreFloat4(&mRotation, identity);

	BuildBoxGeometry();
}

OBB::OBB(const XMFLOAT3 size)
{
	mSize = size;
	mPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMVECTOR identity = XMQuaternionIdentity();
	XMStoreFloat4(&mRotation, identity);



	BuildBoxGeometry();
}

OBB::~OBB()
{
}

bool OBB::Intersect(OBB& Volume)
{
    XMVECTOR posA = XMLoadFloat3(&Volume.mPosition);
    XMVECTOR posB = XMLoadFloat3(&mPosition);

    XMVECTOR rotA = XMLoadFloat4(&mRotation);
    XMVECTOR rotB = XMLoadFloat4(&Volume.mRotation);

    XMVECTOR tmp = posA - posB;
    float length = sqrtf(XMVectorGetX(tmp) * 2 + XMVectorGetY(tmp) * 2 + XMVectorGetZ(tmp) * 2);
    if (length > (mRadius + Volume.mRadius))
        return false;

    XMMATRIX r1 = XMMatrixRotationQuaternion(rotA);
    r1 = XMMatrixInverse(nullptr, r1);
    XMMATRIX r2 = XMMatrixRotationQuaternion(rotB);
    XMMATRIX matB = r2 * r1;

	XMFLOAT4 vPosB;
	XMStoreFloat4(&vPosB, XMVector3Transform((posA - posB), r1));

    XMVECTOR x = XMVectorSet(matB.r[0].m128_f32[0], matB.r[1].m128_f32[0], matB.r[2].m128_f32[0], 0.0f);
    XMVECTOR y = XMVectorSet(matB.r[0].m128_f32[1], matB.r[1].m128_f32[1], matB.r[2].m128_f32[1], 0.0f);
    XMVECTOR z = XMVectorSet(matB.r[0].m128_f32[2], matB.r[1].m128_f32[2], matB.r[2].m128_f32[2], 0.0f);

	XMFLOAT4 XAxis, YAxis, ZAxis;
	XMStoreFloat4(&XAxis, x);
	XMStoreFloat4(&YAxis, y);
	XMStoreFloat4(&ZAxis, z);


	//15 tests
	//1 (Ra)x
	if (fabs(vPosB.x) > mSize.x + Volume.mSize.x * fabs(XAxis.x) + Volume.mSize.y * fabs(XAxis.y) + Volume.mSize.z * fabs(XAxis.z))
		return false;
	//2 (Ra)y
	if (fabs(vPosB.y) > mSize.y + Volume.mSize.x * fabs(YAxis.x) + Volume.mSize.y * fabs(YAxis.y) + Volume.mSize.z * fabs(YAxis.z))
		return false;
	//3 (Ra)z
	if (fabs(vPosB.z) > mSize.z + Volume.mSize.x * fabs(ZAxis.x) + Volume.mSize.y * fabs(ZAxis.y) + Volume.mSize.z * fabs(ZAxis.z))
		return false;

	//4 (Rb)x
	if (fabs(vPosB.x * XAxis.x + vPosB.y * YAxis.x + vPosB.z * ZAxis.x) > (Volume.mSize.x + mSize.x * fabs(XAxis.x) + mSize.y * fabs(YAxis.x) + mSize.z * fabs(ZAxis.x)))
		return false;
	//5 (Rb)y
	if (fabs(vPosB.x * XAxis.y + vPosB.y * YAxis.y + vPosB.z * ZAxis.y) > (Volume.mSize.y + mSize.x * fabs(XAxis.y) + mSize.y * fabs(YAxis.y) + mSize.z * fabs(ZAxis.y)))
		return false;
	//6 (Rb)z
	if (fabs(vPosB.x * XAxis.z + vPosB.y * YAxis.z + vPosB.z * ZAxis.z) > (Volume.mSize.z + mSize.x * fabs(XAxis.z) + mSize.y * fabs(YAxis.z) + mSize.z * fabs(ZAxis.z)))
		return false;

	//7 (Ra)x X (Rb)x
	if (fabs(vPosB.z * YAxis.x - vPosB.y * ZAxis.x) > mSize.y * fabs(ZAxis.x) + mSize.z * fabs(YAxis.x) + Volume.mSize.y * fabs(XAxis.z) + Volume.mSize.z * fabs(XAxis.y))
		return false;
	//8 (Ra)x X (Rb)y
	if (fabs(vPosB.z * YAxis.y - vPosB.y * ZAxis.y) > mSize.y * fabs(ZAxis.y) + mSize.z * fabs(YAxis.y) + Volume.mSize.x * fabs(XAxis.z) + Volume.mSize.z * fabs(XAxis.x))
		return false;
	//9 (Ra)x X (Rb)z
	if (fabs(vPosB.z * YAxis.z - vPosB.y * ZAxis.z) > mSize.y * fabs(ZAxis.z) + mSize.z * fabs(YAxis.z) + Volume.mSize.x * fabs(XAxis.y) + Volume.mSize.y * fabs(XAxis.x))
		return false;

	//10 (Ra)y X (Rb)x
	if (fabs(vPosB.x * ZAxis.x - vPosB.z * XAxis.x) > mSize.x * fabs(ZAxis.x) + mSize.z * fabs(XAxis.x) + Volume.mSize.y * fabs(YAxis.z) + Volume.mSize.z * fabs(YAxis.y))
		return false;
	//11 (Ra)y X (Rb)y
	if (fabs(vPosB.x * ZAxis.y - vPosB.z * XAxis.y) > mSize.x * fabs(ZAxis.y) + mSize.z * fabs(XAxis.y) + Volume.mSize.x * fabs(YAxis.z) + Volume.mSize.z * fabs(YAxis.x))
		return false;
	//12 (Ra)y X (Rb)z
	if (fabs(vPosB.x * ZAxis.z - vPosB.z * XAxis.z) > mSize.x * fabs(ZAxis.z) + mSize.z * fabs(XAxis.z) + Volume.mSize.x * fabs(YAxis.y) + Volume.mSize.y * fabs(YAxis.x))
		return false;

	//13 (Ra)z X (Rb)x
	if (fabs(vPosB.y * XAxis.x - vPosB.x * YAxis.x) > mSize.x * fabs(YAxis.x) + mSize.y * fabs(XAxis.x) + Volume.mSize.y * fabs(ZAxis.z) + Volume.mSize.z * fabs(ZAxis.y))
		return false;
	//14 (Ra)z X (Rb)y
	if (fabs(vPosB.y * XAxis.y - vPosB.x * YAxis.y) > mSize.x * fabs(YAxis.y) + mSize.y * fabs(XAxis.y) + Volume.mSize.x * fabs(ZAxis.z) + Volume.mSize.z * fabs(ZAxis.x))
		return false;
	//15 (Ra)z X (Rb)z
	if (fabs(vPosB.y * XAxis.z - vPosB.x * YAxis.z) > mSize.x * fabs(YAxis.z) + mSize.y * fabs(XAxis.z) + Volume.mSize.x * fabs(ZAxis.y) + Volume.mSize.y * fabs(ZAxis.x))
		return false;

	return true;
}



void OBB::Render()
{
	auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	
	UINT stride = sizeof(SimpleVertex::Mesh);
	UINT offset = 0;

	pd3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	pd3dImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	pd3dImmediateContext->DrawIndexed(mIndexCount, 0, 0);
}





void OBB::BuildBoxGeometry()
{
	auto pd3dDevice = DXUTGetD3D11Device();

	GeometryGenerator geoGen;
	geoGen.CreateBox(mSize.x * 2.0f, mSize.y * 2.0f, mSize.z * 2.0f, mBox);
	mRadius = sqrtf(mSize.x * mSize.x + mSize.y * mSize.y + mSize.z * mSize.z);

	std::vector <SimpleVertex::Mesh> vertices(mBox.Vertices.size());
	UINT k = 0;
	for (UINT i = 0; i < mBox.Vertices.size(); i++, k++)
	{
		vertices[k].Pos = mBox.Vertices[i].Position;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(SimpleVertex::Mesh) * mBox.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HRESULT hr = (pd3dDevice->CreateBuffer(&vbd, &vinitData, &mVertexBuffer));


	std::vector <UINT> indices;
	indices.insert(indices.end(), mBox.Indices.begin(), mBox.Indices.end());
	mIndexCount = mBox.Indices.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mBox.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	hr = (pd3dDevice->CreateBuffer(&ibd, &iinitData, &mIndexBuffer));
}
