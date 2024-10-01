#pragma once

#include <DXUT.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <vector>
#include <unordered_map>
#include "Vertex.h"

using namespace DirectX;

struct VertexWeight 
{
	UINT VertexID;   
	float WeightValue;           
};

struct Bone 
{
	std::string Name;                
	XMFLOAT4X4 OffsetMatrix;     
	std::vector<VertexWeight> weights;  
	std::vector<Bone*> children;

	XMFLOAT3 GetBonePosition();
};


class MeshGenerator
{
public:
	MeshGenerator(ID3D11Device* pd3dDevice, const std::string& filename);
	~MeshGenerator();

	std::vector<ID3D11Buffer*> GetVertexBuffer() const;
	std::vector<ID3D11Buffer*> GetIndexBuffer() const;
	std::vector<UINT>		   GetIndexCount() const;
	std::vector<Bone*>		   GetBones() const;

	void Draw(ID3D11DeviceContext* pd3dImmediateContext, UINT stride, UINT offset, size_t i);
	void RenderSkeleton(Bone* bone, Bone* parent, XMFLOAT4X4 world);

private:
	bool LoadMesh(ID3D11Device* pd3dDevice, const std::string& filename);
	void ReadVetices(ID3D11Device* pd3dDevice, const aiScene* scene);
	void ReadIndices(ID3D11Device* pd3dDevice, const aiScene* scene);
	void ReadSkeletal(ID3D11Device* pd3dDevice, const aiScene* scene);

	std::vector<Bone*> bones;

	std::vector<ID3D11Buffer*>				mVertexBuffer;
	std::vector<ID3D11Buffer*>				mIndexBuffer;	
	std::vector<UINT>						mIndexCounts;
	std::vector<ID3D11ShaderResourceView*>	mSRV;
};