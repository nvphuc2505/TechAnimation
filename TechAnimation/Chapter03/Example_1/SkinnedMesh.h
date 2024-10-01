#pragma once

#include <DXUT.h>
#include <SDKmesh.h>
#include <DXUTcamera.h>
#include <string>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "Vertex.h"
#include "Common/GeometryGenerator.h"

using namespace DirectX;

namespace SkinnedVertex
{
	struct Bone
	{
		Bone(XMFLOAT3 Position, XMFLOAT4 Col)
		{
			Pos = Position;
			Color = Col;
		}

		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};
}

// struct for storing bones (joints)
struct Bone
{
	std::string Name;
	XMFLOAT4X4 TransformationMatrix;			// Local bone pos, rot & sca
	XMFLOAT4X4 CombinedTransformationMatrix;	// the actual world transformation of a specific bone

	Bone* pFrameFirstChild; 
	Bone* pFrameSibling;
};

class SkinnedMesh
{
public:
	SkinnedMesh(ID3D11Device* pd3dDevice, const std::string& filename);
	~SkinnedMesh();
	void Load(const std::string& filename);
	void RenderSkeleton(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, 
		Bone* bone, Bone* parent, XMMATRIX world);

// private:
	Bone* ReadSkeleton(const aiNode* node);
	void UpdateMatrices(Bone* bone, XMFLOAT4X4& parentMatrix);

	ID3D11Buffer* mSphereVB;
	ID3D11Buffer* mSphereIB;
	ID3D11Buffer* mLineVertexBuffer;
	Bone* mRootBone;
	GeometryGenerator::MeshData sphere;
};