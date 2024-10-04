#include "SkinnedMesh.h"

SkinnedModel::SkinnedModel()
{

}

SkinnedModel::SkinnedModel(const std::string& filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate);

	LoadModel(scene);
}

SkinnedModel::~SkinnedModel()
{
	for(auto vb : mVertexBuffers)
		SAFE_RELEASE(vb);

	for (auto ib : mVertexBuffers)
		SAFE_RELEASE(ib);
}





void SkinnedModel::LoadModel(const aiScene* scene, std::vector<SkinnedVertex::Mesh>& verticesOutput, std::vector<UINT>& indicesOutput, Bone& skeletonOutput, UINT& nBoneCount)
{
	if (!scene || scene->mNumMeshes <= 0)
	{
		OutputDebugStringA("ERROR: scene is nullptr\n");
		return;
	}

	UINT NumMesh = scene->mNumMeshes;
	for (UINT m = 0; m < NumMesh; m++)
	{
		aiMesh* mesh = scene->mMeshes[m];

		ReadVertices(mesh);
		ReadIndices(mesh);
	}
}





std::pair<UINT, float> SkinnedModel::GetTimeFraction(std::vector<float>& times, float& dt)
{
	float totalDuration = times.back();
	dt = fmod(dt, totalDuration);

	if (dt <= times.front())
		return { 0, 0.0f };

	if (dt >= times.back())
		return { static_cast<UINT>(times.size() - 1), 0.0f };

	UINT segment = 1;
	while (dt > times[segment])
		segment++;

	float start = times[segment - 1];
	float end = times[segment];
	float frac = (dt - start) / (end - start);

	return { segment, frac };
}
void SkinnedModel::ReadVertices(const aiMesh* mesh)
{
	if (!mesh)
	{
		OutputDebugStringA("ERROR: mesh is nullptr\n");
		return;
	}

	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		SkinnedVertex::Mesh vertex;

		// Process Position
		vertex.Pos.x = mesh->mVertices[i].x;
		vertex.Pos.y = mesh->mVertices[i].y;
		vertex.Pos.z = mesh->mVertices[i].z;

		// Process Normal
		vertex.Normal.x = mesh->mNormals[i].x;
		vertex.Normal.y = mesh->mNormals[i].y;
		vertex.Normal.z = mesh->mNormals[i].z;

		// Process Texcoordinate
		vertex.Tex.x = mesh->mTextureCoords[0][i].x;
		vertex.Tex.y = mesh->mTextureCoords[0][i].y;

		mVertices.push_back(vertex);
	}


	// Process BoneID and its Weight
	std::unordered_map <std::string, std::pair <int, XMFLOAT4X4>> SampleBoneInfo = {};
	std::vector<UINT> SampleBoneCounts;
	SampleBoneCounts.resize(mVertices.size());
	this->mBoneCount = mesh->mNumBones;

	for (UINT id = 0; id < mesh->mNumBones; id++)
	{
		aiBone* AssimpBone = mesh->mBones[id];	
		XMFLOAT4X4 OffsetMatrix = Utils::AssimpToXMFLOAT4X4(AssimpBone->mOffsetMatrix);
		SampleBoneInfo[AssimpBone->mName.C_Str()] = { id, OffsetMatrix };

		for (UINT j = 0; j < AssimpBone->mNumWeights; j++)
		{
			UINT id = AssimpBone->mWeights[j].mVertexId;
			float weight = AssimpBone->mWeights[j].mWeight;
			SampleBoneCounts[id]++;

			switch (SampleBoneCounts[id])
			{
				case 1:
					mVertices[id].BoneID[0] = id;
					mVertices[id].BoneWeight.x = weight;
					break;
				case 2:
					mVertices[id].BoneID[1] = id;
					mVertices[id].BoneWeight.y = weight;
					break;
				case 3:
					mVertices[id].BoneID[2] = id;
					mVertices[id].BoneWeight.z = weight;
					break;
				case 4:
					mVertices[id].BoneID[3] = id;
					mVertices[id].BoneWeight.w = weight;
					break;
				default:
					break;	
			}
		}
	}

	// Normalize weights to make all weights sum 1
	for (int i = 0; i < this->mVertices.size(); i++)
	{
		XMFLOAT4& BoneWeight = this->mVertices[i].BoneWeight;

		float TotalWeight = BoneWeight.x + BoneWeight.y + BoneWeight.z + BoneWeight.w;
		if (TotalWeight > 0.0f)
		{
			this->mVertices[i].BoneWeight = XMFLOAT4
			(
				BoneWeight.x / TotalWeight,
				BoneWeight.y / TotalWeight,
				BoneWeight.z / TotalWeight,
				BoneWeight.w / TotalWeight
			);
		}
	}

	
	// Initialize Vertex Buffer
	auto pd3dDevice = DXUTGetD3D11Device();

	ID3D11Buffer* VertexBuffer;
	D3D11_BUFFER_DESC vbd = {};
	vbd.ByteWidth = sizeof(SkinnedVertex::Mesh) * mVertices.size();
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vInitData = {};
	vInitData.pSysMem = mVertices.data();
	HRESULT hr = pd3dDevice->CreateBuffer(&vbd, &vInitData, &VertexBuffer);

	mVertexBuffers.push_back(VertexBuffer);
	SAFE_RELEASE(VertexBuffer);
}
void SkinnedModel::ReadIndices(const aiMesh* mesh)
{
	if (!mesh)
	{
		OutputDebugStringA("ERROR: mesh is nullptr\n");
		return;
	}

	// Process Index
	mIndices.reserve(mesh->mNumFaces * 3);
	if(mesh->HasFaces())
	{
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			aiFace& face = mesh->mFaces[i];

			for (UINT j = 0; j < face.mNumIndices; j++)
				mIndices.push_back(face.mIndices[j]);
		}
	}

	UINT cnt = mIndices.size();
	mIndicesCount.push_back(cnt);


	// Initialize Index Buffers 
	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(unsigned int) * cnt;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA iInitData = {};
	iInitData.pSysMem = mIndices.data();

	auto pd3dDevice = DXUTGetD3D11Device();
	ID3D11Buffer* indexBuffer;
	HRESULT hr = pd3dDevice->CreateBuffer(&ibd, &iInitData, &indexBuffer);
	mIndexBuffers.push_back(indexBuffer);
	SAFE_RELEASE(indexBuffer);
}
