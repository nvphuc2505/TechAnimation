#pragma once

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <vector>
#include "Vertex.h"


class MeshGenerator
{
public:
	MeshGenerator(ID3D11Device* pd3dDevice, const std::string& filename);
	~MeshGenerator();

	std::vector<ID3D11Buffer*> GetVertexBuffer() const;
	std::vector<ID3D11Buffer*> GetIndexBuffer() const;
	std::vector<UINT>		   GetIndexCount() const;

	void Draw(ID3D11DeviceContext* pd3dImmediateContext, UINT stride, UINT offset, size_t i);

private:
	bool LoadMesh(ID3D11Device* pd3dDevice, const std::string& filename);
	void ReadVetices(ID3D11Device* pd3dDevice, const aiScene* scene);
	void ReadIndices(ID3D11Device* pd3dDevice, const aiScene* scene);


	std::vector<ID3D11Buffer*>				mVertexBuffer;
	std::vector<ID3D11Buffer*>				mIndexBuffer;	
	std::vector<UINT>						mIndexCounts;
	std::vector<ID3D11ShaderResourceView*>	mSRV;
};