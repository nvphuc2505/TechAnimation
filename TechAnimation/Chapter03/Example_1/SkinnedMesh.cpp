#include "SkinnedMesh.h"


XMFLOAT4X4 AssimpToXMFLOAT4X4(const aiMatrix4x4& aiMat)
{
	return XMFLOAT4X4
	(
		aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
		aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
		aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
		aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
	);
}

void GetLocalTransform(const XMFLOAT4X4& matrix, XMFLOAT3& position, XMFLOAT3& rotation, XMFLOAT3& scale) 
{
	XMMATRIX mat = XMLoadFloat4x4(&matrix);

	position = { XMVectorGetX(mat.r[3]),
		XMVectorGetY(mat.r[3]),
		XMVectorGetZ(mat.r[3]) };

	scale.x = XMVectorGetX(XMVector3Length(mat.r[0]));
	scale.y = XMVectorGetX(XMVector3Length(mat.r[1]));
	scale.z = XMVectorGetX(XMVector3Length(mat.r[2]));

	mat.r[0] = XMVector3Normalize(mat.r[0]);
	mat.r[1] = XMVector3Normalize(mat.r[1]);
	mat.r[2] = XMVector3Normalize(mat.r[2]);

	rotation.y = atan2f(mat.r[2].m128_f32[0], mat.r[2].m128_f32[2]); 
	rotation.x = asinf(-mat.r[2].m128_f32[1]); 
	rotation.z = atan2f(mat.r[1].m128_f32[1], mat.r[0].m128_f32[1]); 
}



SkinnedMesh::SkinnedMesh(ID3D11Device* pd3dDevice, const std::string& filename)
{
    Load(filename);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC; 
	bufferDesc.ByteWidth = sizeof(SkinnedVertex::Bone) * 2;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &mLineVertexBuffer);

	GeometryGenerator geoGen;
	geoGen.CreateSphere(10.0f, 10, 10, sphere);
}

SkinnedMesh::~SkinnedMesh()
{

}

void SkinnedMesh::Load(const std::string& filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

	XMFLOAT4X4 M;
	XMStoreFloat4x4(&M, XMMatrixIdentity());
	mRootBone = ReadSkeleton(scene->mRootNode);
	UpdateMatrices(mRootBone, M);
	if (!mRootBone)
		OutputDebugStringA("mRootBone is nullptr");


	
}

void SkinnedMesh::RenderSkeleton(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
	Bone* bone, Bone* parent, XMMATRIX world)
{
	if (!bone) bone = mRootBone;

	if (parent && !parent->Name.empty() && !bone->Name.empty())
	{
		XMFLOAT4X4 w1 = bone->CombinedTransformationMatrix;
		XMFLOAT4X4 w2 = parent->CombinedTransformationMatrix;

		XMFLOAT3 bonePos = { w1.m[3][0], w1.m[3][1], w1.m[3][2] };
		XMFLOAT3 parentPos = { w2.m[3][0], w2.m[3][1], w2.m[3][2] };
		
			SkinnedVertex::Bone vertices[] =
			{
				SkinnedVertex::Vertex(parentPos, XMFLOAT4(1, 0, 0, 1)), 
				SkinnedVertex::Vertex(bonePos, XMFLOAT4(0, 0, 1, 1))    
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

			if (distance < 2.0f)
			{
				UINT stride = sizeof(SkinnedVertex::Bone);
				UINT offset = 0;
				pd3dImmediateContext->IASetVertexBuffers(0, 1, &mLineVertexBuffer, &stride, &offset);
				pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
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
	bone->TransformationMatrix = AssimpToXMFLOAT4X4(node->mTransformation);
	bone->pFrameSibling = nullptr;
	bone->pFrameFirstChild = nullptr;

	XMFLOAT3 position, rotation, scale;
	GetLocalTransform(bone->TransformationMatrix, position, rotation, scale);
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

	if (node->mNumChildren > 0) {
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
