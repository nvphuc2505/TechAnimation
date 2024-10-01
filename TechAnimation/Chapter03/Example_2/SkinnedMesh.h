#pragma once

#include <DXUT.h>
#include <SDKmesh.h>
#include <DXUTcamera.h>
#include <string>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "Utils.h"
#include "Vertex.h"
#include "RenderState.h"
#include "Common/GeometryGenerator.h"

using namespace DirectX;



// storing bones (JOINTS)
struct Bone
{
	std::string				Name;
	XMFLOAT4X4				TransformationMatrix;			
	XMFLOAT4X4				CombinedTransformationMatrix;
	// BoneMesh*			pBoneMeshContainer;
	Bone*					pFrameSibling;
	Bone*					pFrameFirstChild; 
};


/*
struct BoneMesh
{
	std::string Name;

	XMFLOAT4X4 BoneOffsetMatrix;
	XMFLOAT4X4 CurrentBoneOffsetMatrix;
	BoneMesh* pNextMeshContainer;
};
*/



class SkinnedMesh
{
public:
	SkinnedMesh(ID3D11Device* pd3dDevice, const std::string& filename);
	~SkinnedMesh();
	void Load(ID3D11Device* pd3dDevice, const std::string& filename);
	void Render(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext);
	void RenderSkeleton(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, 
		Bone* bone, Bone* parent, XMMATRIX world);

private:
	Bone* ReadSkeleton(const aiNode* node); // Node
	void ReadVertices(ID3D11Device* pd3dDevice, const aiScene* scene);
	void ReadIndices(ID3D11Device* pd3dDevice, const aiScene* scene);
	void UpdateMatrices(Bone* bone, XMFLOAT4X4& parentMatrix);

	Bone* mRootBone;
	ID3D11Buffer* mLineVertexBuffer;

	std::vector <ID3D11Buffer*> mVertexBuffer;
	std::vector <ID3D11Buffer*> mIndexBuffer;
	std::vector <UINT>			mIndexCounts;

};