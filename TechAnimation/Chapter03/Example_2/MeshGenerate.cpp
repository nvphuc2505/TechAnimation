#include "MeshGenerate.h"



XMFLOAT4X4 ConvertAiMatrixToXMMATRIX(const aiMatrix4x4& matrix) {
	return XMFLOAT4X4(
		matrix.a1, matrix.b1, matrix.c1, matrix.d1,
		matrix.a2, matrix.b2, matrix.c2, matrix.d2,
		matrix.a3, matrix.b3, matrix.c3, matrix.d3,
		matrix.a4, matrix.b4, matrix.c4, matrix.d4
	);
}

MeshGenerator::MeshGenerator(ID3D11Device* pd3dDevice, const std::string& filename)
{
	LoadMesh(pd3dDevice, filename);
}

MeshGenerator::~MeshGenerator()
{
}



void MeshGenerator::ReadVetices(ID3D11Device* pd3dDevice, const aiScene* scene)
{
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		const aiMesh* mesh = scene->mMeshes[m];

		std::vector<Vertex::Basic32> vertices(mesh->mNumVertices);
		if (mesh->HasPositions())
		{
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				aiVector3D* vp = &(mesh->mVertices[i]);
				vertices[i].Position = XMFLOAT3(vp->x, vp->y, vp->z);
			}
		}
		if (mesh->HasNormals())
		{
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				const aiVector3D* vn = &(mesh->mNormals[i]);
				vertices[i].Normal = XMFLOAT3(vn->x, vn->y, vn->z);
			}
		}
		if (mesh->HasTextureCoords(0))
		{
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				const aiVector3D* vt = &(mesh->mTextureCoords[0][i]);
				vertices[i].Tex = XMFLOAT2(vt->x, vt->y);
			}
		}

		D3D11_BUFFER_DESC vbd = {};
		vbd.ByteWidth = sizeof(Vertex::Basic32) * vertices.size();
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA vInitData = {};
		vInitData.pSysMem = vertices.data();

		ID3D11Buffer* vertexBuffer;
		HRESULT hr = pd3dDevice->CreateBuffer(&vbd, &vInitData, &vertexBuffer);
		mVertexBuffer.push_back(vertexBuffer);
	}
}

void MeshGenerator::ReadIndices(ID3D11Device* pd3dDevice, const aiScene* scene)
{
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		const aiMesh* mesh = scene->mMeshes[m];
		std::vector<unsigned int> indices;
		indices.reserve(mesh->mNumFaces * 3);
		if (mesh->HasFaces())
		{
			for (unsigned int j = 0; j < mesh->mNumFaces; j++)
			{
				aiFace face = mesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; k++)
				{
					indices.push_back(face.mIndices[k]);
				}
			}
		}

		UINT indexCount = indices.size();
		mIndexCounts.push_back(indexCount);

		D3D11_BUFFER_DESC ibd = {};
		ibd.ByteWidth = sizeof(unsigned int) * indexCount;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA iInitData = {};
		iInitData.pSysMem = indices.data();

		ID3D11Buffer* indexBuffer;
		HRESULT hr = pd3dDevice->CreateBuffer(&ibd, &iInitData, &indexBuffer);
		mIndexBuffer.push_back(indexBuffer);
	}
}

void MeshGenerator::ReadSkeletal(ID3D11Device* pd3dDevice, const aiScene* scene)
{
	std::unordered_map<std::string, Bone*> boneMap;

	for (UINT m = 0; m < scene->mNumMeshes; ++m)
	{
		const aiMesh* mesh = scene->mMeshes[m];
		
		if (mesh->HasBones())
		{
			for (int i = 0; i < mesh->mNumBones; i++)
			{
				aiBone* SampleBone = mesh->mBones[i];

				Bone* NewBone = new Bone();
				NewBone->Name = SampleBone->mName.C_Str();
				NewBone->OffsetMatrix = ConvertAiMatrixToXMMATRIX(SampleBone->mOffsetMatrix);
				
				for(UINT j = 0; j < SampleBone->mNumWeights; j++)
				{
					aiVertexWeight weight = SampleBone->mWeights[j];

					VertexWeight NewWeight;
					NewWeight.VertexID = weight.mVertexId;
					NewWeight.WeightValue = weight.mWeight;

					NewBone->weights.push_back(NewWeight);
				}

				boneMap[NewBone->Name] = NewBone;
				bones.push_back(NewBone);
			}
		}
	}

	for (UINT m = 0; m < scene->mNumMeshes; ++m)
	{
		const aiMesh* mesh = scene->mMeshes[m];
		for (UINT i = 0; i < mesh->mNumBones; i++)
		{
			aiBone* SampleBone = mesh->mBones[i];
			Bone* currentBone = boneMap[SampleBone->mName.C_Str()];
			std::string parentName = currentBone->Name;

			if (boneMap.find(parentName) != boneMap.end())
			{
				Bone* parentBone = boneMap[parentName];
				parentBone->children.push_back(currentBone);
			}
		}
	}
}


void MeshGenerator::Draw(ID3D11DeviceContext* pd3dImmediateContext,
	UINT stride, UINT offset,
	size_t i)
{
	pd3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer[i], &stride, &offset);
	pd3dImmediateContext->IASetIndexBuffer(mIndexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

	pd3dImmediateContext->DrawIndexed(mIndexCounts[i], 0, 0);
}





bool MeshGenerator::LoadMesh(ID3D11Device* pd3dDevice, const std::string& filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (scene && !(scene->mNumMeshes <= 0))
	{
		ReadVetices(pd3dDevice, scene);
		ReadIndices(pd3dDevice, scene);
		ReadSkeletal(pd3dDevice, scene);

		return true;
	}

	return false;
}



std::vector<ID3D11Buffer*> MeshGenerator::GetVertexBuffer() const
{
	return mVertexBuffer;
}

std::vector<ID3D11Buffer*> MeshGenerator::GetIndexBuffer() const
{
	return mIndexBuffer;
}

std::vector<UINT> MeshGenerator::GetIndexCount() const
{
	return mIndexCounts;
}

std::vector<Bone*> MeshGenerator::GetBones() const
{
	return bones;
}

XMFLOAT3 Bone::GetBonePosition()
{
	XMVECTOR position = XMLoadFloat4x4(&OffsetMatrix).r[3]; // Get the translation vector from the OffsetMatrix
	XMFLOAT3 bonePosition;
	XMStoreFloat3(&bonePosition, position); // Store the position in a float3
	return bonePosition;
}
