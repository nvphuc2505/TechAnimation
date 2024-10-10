#pragma once

#include <DXUT.h>
#include <btBulletDynamicsCommon.h>
#include "OBB.h"	

class PhysicsEngine
{
public:
	PhysicsEngine();
	~PhysicsEngine();

	void Init();
	void Release();
	void Update(float deltaTime);
	void Render(ID3D11Buffer* g_pCBChangesEveryFrame, CModelViewerCamera g_Camera);
	void Reset();
	OBB* CreateOBB(XMFLOAT3 pos, XMFLOAT3 size);

private:
	XMFLOAT3 RandomVector(XMFLOAT3& min, XMFLOAT3& max);
	float RandomFloat();

	std::vector<OBB*> m_boxes;
	float m_time;

	btDynamicsWorld* m_dynamicsWorld;
	btRigidBody* m_floor;
};