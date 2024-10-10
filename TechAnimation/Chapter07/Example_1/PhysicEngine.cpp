#include "PhysicEngine.h"


PhysicsEngine::PhysicsEngine()
{

}

PhysicsEngine::~PhysicsEngine()
{
	Release();
}



void PhysicsEngine::Init()
{
	srand(GetTickCount());

	// Default Collision Configuration
	btCollisionConfiguration* collisionConfiguration;
	collisionConfiguration = new btDefaultCollisionConfiguration();

	// Default Constraint Solver
	btConstraintSolver* constraintSolver;
	constraintSolver = new btSequentialImpulseConstraintSolver();

	btVector3 worldAabbMin(-1000, -1000, -1000);
	btVector3 worldAabbMax(1000, 1000, 1000);
	const int maxProxies = 32766;
	btBroadphaseInterface* pairCache;
	pairCache = new btAxisSweep3(worldAabbMin, worldAabbMax, maxProxies);

	// Dispatcher
	btDispatcher* dispatcher;
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	m_dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration);

	//Create "floor" plane
	btStaticPlaneShape* floorShape = new btStaticPlaneShape(btVector3(0, 1, 0), -10.0f);
	btDefaultMotionState* motionState = new btDefaultMotionState();
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0.0f, motionState, floorShape);
	m_floor = new btRigidBody(rigidBodyCI);
	m_dynamicsWorld->addRigidBody(m_floor);

	Reset();
}

void PhysicsEngine::Release()
{
	if (!m_dynamicsWorld)
		delete m_dynamicsWorld;
}

void PhysicsEngine::Update(float deltaTime)
{
	if (m_dynamicsWorld != nullptr)
		m_dynamicsWorld->stepSimulation(deltaTime);
}

void PhysicsEngine::Render(ID3D11Buffer* g_pCBChangesEveryFrame, CModelViewerCamera g_Camera)
{
	for (auto box : m_boxes)
		box->Render(g_pCBChangesEveryFrame, g_Camera);
}

void PhysicsEngine::Reset()
{
	// Remove old boxes
	for(int i = 0; i < m_boxes.size(); i++)
	{
		m_dynamicsWorld->removeRigidBody(m_boxes[i]->mBody);
		delete m_boxes[i];
	}
	m_boxes.clear();

	// Create new boxes
	for (int i = 0; i < 100; i++)
	{
		XMFLOAT3 minPos = XMFLOAT3(-10.0f, 0.0f, -10.0f);
		XMFLOAT3 maxPos = XMFLOAT3(10.0f, 50.0f, 10.0f);
		XMFLOAT3 position = RandomVector(minPos, maxPos);

		XMFLOAT3 minSize = XMFLOAT3(0.5f, 0.5f, 0.5f);
		XMFLOAT3 maxSize = XMFLOAT3(3.0f, 3.0f, 3.0f);
		XMFLOAT3 size = RandomVector(minSize, maxSize);

		OBB* newBox = CreateOBB(position, size);
		m_boxes.push_back(newBox);
	}
}

OBB* PhysicsEngine::CreateOBB(XMFLOAT3 pos, XMFLOAT3 size)
{
	OBB* newOBB = new OBB(pos, size);
	m_dynamicsWorld->addRigidBody(newOBB->mBody);
	
	return newOBB;
}



//
// Random Function
//
XMFLOAT3 PhysicsEngine::RandomVector(XMFLOAT3& min, XMFLOAT3& max)
{
	return XMFLOAT3(RandomFloat() * (max.x - min.x) + min.x,
					RandomFloat() * (max.y - min.y) + min.y,
					RandomFloat() * (max.z - min.z) + min.z);
}

float PhysicsEngine::RandomFloat()
{
	return (rand() % 1000) / 1000.0f;
}
