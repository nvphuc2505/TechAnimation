#pragma once
#include <DXUT.h>
#include <DXUTcamera.h>
#include <btBulletDynamicsCommon.h>
#include "Common/MathHelper.h"
#include "Common/GeometryGenerator.h"
#include "Vertex.h"


using namespace DirectX;

struct Ray
{
	XMFLOAT3 mOrigin;
	XMFLOAT3 mDirection;
};

// Oriented Bounding Boxes
class OBB
{
public:
	OBB();
	OBB(const XMFLOAT3 size);
	OBB(XMFLOAT3 pos, XMFLOAT3 size, bool dynamic = true);
	~OBB();

	// bool Intersect(OBB& Volume);
	void Render(ID3D11Buffer* g_pCBChangesEveryFrame, CModelViewerCamera g_Camera);

public:
	GeometryGenerator::MeshData mBox;
	XMFLOAT3 mSize;
	XMFLOAT3 mPosition;
	XMFLOAT4 mRotation;
	float mRadius;

	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;
	UINT mIndexCount;

	btRigidBody* mBody;

private:
	void BuildBoxGeometry();
};