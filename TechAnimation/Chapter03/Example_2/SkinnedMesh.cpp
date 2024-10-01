#include "SkinnedMesh.h"







SkinnedMesh::SkinnedMesh(ID3D11Device* pd3dDevice, const std::string& filename)
{
	Load(pd3dDevice, filename);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(SkinnedVertex::Bone) * 2;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &mLineVertexBuffer);
}

SkinnedMesh::~SkinnedMesh()
{
	
}

void SkinnedMesh::Load(ID3D11Device* pd3dDevice, const std::string& filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);
	if(!scene)
		OutputDebugStringA("aiScene is nullptr");

	XMFLOAT4X4 M;
	XMStoreFloat4x4(&M, XMMatrixIdentity());
	
	mRootBone = ReadSkeleton(scene->mRootNode);
	if (!mRootBone)
		OutputDebugStringA("mRootBone is nullptr");

	UpdateMatrices(mRootBone, M);

	ReadVertices(pd3dDevice, scene);
	ReadIndices(pd3dDevice, scene);
}

void SkinnedMesh::Render(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext)
{
	UINT stride = sizeof(SkinnedVertex::Mesh);
	UINT offset = 0;
	
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	for (size_t i = 0; i < mVertexBuffer.size(); ++i)
	{
		pd3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer[i], &stride, &offset);
		pd3dImmediateContext->IASetIndexBuffer(mIndexBuffer[i], DXGI_FORMAT_R32_UINT, 0);
		
		if(GetAsyncKeyState(VK_SPACE))
		{
			pd3dImmediateContext->RSSetState(RenderState::mWireframe);
			pd3dImmediateContext->DrawIndexed(mIndexCounts[i], 0, 0);
			pd3dImmediateContext->RSSetState(0);
		}
		else
		{
			pd3dImmediateContext->DrawIndexed(mIndexCounts[i], 0, 0);
		}
	}
}



void SkinnedMesh::RenderSkeleton(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	Bone* bone, Bone* parent, XMMATRIX world)
{
	if (!bone) 
		bone = mRootBone;

	if (parent && !parent->Name.empty() && !bone->Name.empty())
	{
		XMFLOAT4X4 w1 = bone->CombinedTransformationMatrix;
		XMFLOAT4X4 w2 = parent->CombinedTransformationMatrix;

		XMFLOAT3 bonePos = { w1.m[3][0], w1.m[3][1], w1.m[3][2] };
		XMFLOAT3 parentPos = { w2.m[3][0], w2.m[3][1], w2.m[3][2] };

		SkinnedVertex::Bone vertices[] =
		{
			SkinnedVertex::Bone(parentPos, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)),
			SkinnedVertex::Bone(bonePos, XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f))
		};

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT hr = pd3dImmediateContext->Map(mLineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (SUCCEEDED(hr))
		{
			memcpy(mappedResource.pData, vertices, sizeof(vertices));
			pd3dImmediateContext->Unmap(mLineVertexBuffer, 0);
		}

		XMVECTOR boneVec = XMLoadFloat3(&bonePos);
		XMVECTOR parentVec = XMLoadFloat3(&parentPos);
		XMVECTOR difference = XMVectorSubtract(boneVec, parentVec);
		float distance = XMVectorGetX(XMVector3Length(difference));

		if (distance < 2.5f)
		{
			UINT stride = sizeof(SkinnedVertex::Bone);
			UINT offset = 0;

			pd3dImmediateContext->IASetVertexBuffers(0, 1, &mLineVertexBuffer, &stride, &offset);
			pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
			
			if (GetAsyncKeyState(VK_SPACE))
				pd3dImmediateContext->Draw(2, 0); // Draw 2 vertices
		}
	}

	if (bone->pFrameSibling)RenderSkeleton(pd3dDevice, pd3dImmediateContext, (Bone*)bone->pFrameSibling, parent, world);
	if (bone->pFrameFirstChild)RenderSkeleton(pd3dDevice, pd3dImmediateContext, (Bone*)bone->pFrameFirstChild, bone, world);
}


Bone* SkinnedMesh::ReadSkeleton(const aiNode* node)
{
	if (!node)
	{
		return nullptr;
	}

	Bone* bone = new Bone();
	bone->Name = node->mName.C_Str();
	bone->TransformationMatrix = Utils::AssimpToXMFLOAT4X4(node->mTransformation);
	bone->pFrameSibling = nullptr;
	bone->pFrameFirstChild = nullptr;

	XMFLOAT3 position, rotation, scale;
	Utils::GetLocalTransform(bone->TransformationMatrix, position, rotation, scale);

	/*
	OutputDebugStringA(("Bone Name: " + bone->Name + "\n").c_str());
	OutputDebugStringA(("Bone Position: " + std::to_string(position.x) + ", " +
		std::to_string(position.y) + ", " +
		std::to_string(position.z) + "\n").c_str());
	OutputDebugStringA(("Bone Rotation: " + std::to_string(rotation.x) + ", " +
		std::to_string(rotation.y) + ", " +
		std::to_string(rotation.z) + "\n").c_str());
	OutputDebugStringA(("Bone Scale: " + std::to_string(scale.x) + ", " +
		std::to_string(scale.y) + ", " +
		std::to_string(scale.z) + "\n").c_str());
	*/

	if (node->mNumChildren > 0) 
	{
		bone->pFrameFirstChild = ReadSkeleton(node->mChildren[0]);
		Bone* lastSibling = bone->pFrameFirstChild;

		for (UINT i = 1; i < node->mNumChildren; ++i) {
			Bone* sibling = ReadSkeleton(node->mChildren[i]);
			lastSibling->pFrameSibling = sibling;
			lastSibling = sibling;
		}
	}

	return bone;

}

void SkinnedMesh::ReadVertices(ID3D11Device* pd3dDevice, const aiScene* scene)
{
	if (!scene || !scene->HasMeshes())
		return;

	for (int m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* mesh = scene->mMeshes[m];
		std::vector<SkinnedVertex::Mesh> vertices(mesh->mNumVertices);

		if (mesh->HasPositions())
		{
			for (UINT i = 0; i < mesh->mNumVertices; ++i)
			{
				aiVector3D* vp = &mesh->mVertices[i];
				vertices[i].Pos = XMFLOAT3(vp->x, vp->y, vp->z);
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
		vbd.ByteWidth = sizeof(SkinnedVertex::Mesh) * vertices.size();
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

void SkinnedMesh::ReadIndices(ID3D11Device* pd3dDevice, const aiScene* scene)
{
	if (!scene || !scene->HasMeshes())
		return;

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

		mIndexCounts.push_back(indices.size());

		D3D11_BUFFER_DESC ibd = {};
		ibd.ByteWidth = sizeof(unsigned int) * indices.size();
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

void SkinnedMesh::UpdateMatrices(Bone* bone, XMFLOAT4X4& parentMatrix)
{
	if (!bone)
		return;

	XMMATRIX parentMatrixXM = XMLoadFloat4x4(&parentMatrix);
	XMMATRIX localTransformXM = XMLoadFloat4x4(&bone->TransformationMatrix);

	XMMATRIX globalTransformXM = localTransformXM * parentMatrixXM;
	XMStoreFloat4x4(&bone->CombinedTransformationMatrix, globalTransformXM);

	if (bone->pFrameSibling)
	{
		UpdateMatrices((Bone*)bone->pFrameSibling, parentMatrix);
	}

	if (bone->pFrameFirstChild)
	{
		UpdateMatrices((Bone*)bone->pFrameFirstChild, bone->CombinedTransformationMatrix);
	}
}
