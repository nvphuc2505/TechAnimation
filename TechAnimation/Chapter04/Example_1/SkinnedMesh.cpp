#include "SkinnedMesh.h"



SkinnedModel::SkinnedModel()
{
}

SkinnedModel::SkinnedModel(const aiScene* scene)
{
	LoadModel(scene, scene->mMeshes[0], Vertices, Indices, BoneInfo, BoneCount);
	ExtractAnimation(scene, mAnimation);
}

bool SkinnedModel::ExtractSkeleton(Bone& boneOutput, aiNode* node, std::unordered_map<std::string, std::pair<int, XMFLOAT4X4>>& boneInfoTable)
{
	// if node is actually a bone
	if (boneInfoTable.find(node->mName.C_Str()) != boneInfoTable.end())
	{
		boneOutput.name = node->mName.C_Str();
		boneOutput.id = boneInfoTable[boneOutput.name].first;
		boneOutput.offset = boneInfoTable[boneOutput.name].second;

		for (int i = 0; i < node->mNumChildren; i++) {
			Bone child;
			ExtractSkeleton(child, node->mChildren[i], boneInfoTable);
			boneOutput.children.push_back(child);
		}
		
		return true;
	}
	else
	{
		for (int i = 0; i < node->mNumChildren; i++) 
		{
			if (ExtractSkeleton(boneOutput, node->mChildren[i], boneInfoTable)) 
			{
				return true;
			}

		}
	}

	return false;
}
void SkinnedModel::ExtractAnimation(const aiScene* scene, Animation& animation)
{
	aiAnimation* anim = scene->mAnimations[0];

	if (anim->mTicksPerSecond != 0.0f)
		animation.ticksPerSecond = anim->mTicksPerSecond;
	else
		animation.ticksPerSecond = 1;

	animation.duration = anim->mDuration * anim->mTicksPerSecond;
	animation.boneTransforms = {};

	for (int i = 0; i < anim->mNumChannels; i++) 
	{
		aiNodeAnim* channel = anim->mChannels[i];
		BoneTransformTrack track;

		for (int j = 0; j < channel->mNumPositionKeys; j++) 
		{
			track.positionTimestamps.push_back(channel->mPositionKeys[j].mTime);
			track.positions.push_back(Utils::AssimpToXMFLOAT3(channel->mPositionKeys[j].mValue));
		}

		for (int j = 0; j < channel->mNumRotationKeys; j++) 
		{
			track.rotationTimestamps.push_back(channel->mRotationKeys[j].mTime);
			track.rotations.push_back(Utils::AssimpToXMFLOAT4(channel->mRotationKeys[j].mValue));

		}

		for (int j = 0; j < channel->mNumScalingKeys; j++) 
		{
			track.scaleTimestamps.push_back(channel->mScalingKeys[j].mTime);
			track.scales.push_back(Utils::AssimpToXMFLOAT3(channel->mScalingKeys[j].mValue));

		}

		animation.boneTransforms[channel->mNodeName.C_Str()] = track;
	}
}
void SkinnedModel::LoadModel(const aiScene* scene, aiMesh* mesh, std::vector<SkinnedVertex::Mesh>& verticesOutput, std::vector<UINT>& indicesOutput, Bone& skeletonOutput, UINT& nBoneCount)
{
	verticesOutput = {};
	indicesOutput = {};

	for (UINT i = 0; i < mesh->mNumVertices; i++) 
	{
		//process position 
		SkinnedVertex::Mesh vertex;
		XMFLOAT3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Pos = vector;

		//process normal
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		//process uv
		XMFLOAT2 vec;
		vec.x = mesh->mTextureCoords[0][i].x;
		vec.y = mesh->mTextureCoords[0][i].y;
		vertex.Tex = vec;

		vertex.BoneID;
		vertex.BoneWeight = XMFLOAT4();

		verticesOutput.push_back(vertex);
	}

	std::unordered_map<std::string, std::pair<int, XMFLOAT4X4>> boneInfo = {};
	std::vector<UINT> boneCounts;
	boneCounts.resize(verticesOutput.size(), 0);
	nBoneCount = mesh->mNumBones;

	for (UINT i = 0; i < mesh->mNumBones; i++)
	{
		aiBone* bone = mesh->mBones[i];
		XMFLOAT4X4 m = Utils::AssimpToXMFLOAT4X4(bone->mOffsetMatrix);
		boneInfo[bone->mName.C_Str()] = { i, m };

		//loop through each vertex that have that bone
		for (int j = 0; j < bone->mNumWeights; j++) 
		{
			UINT id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;
			boneCounts[id]++;

			switch (boneCounts[id])
			{
				case 1:
					verticesOutput[id].BoneID[0] = i;
					verticesOutput[id].BoneWeight.x = weight;
					break;
				case 2:
					verticesOutput[id].BoneID[1] = i;
					verticesOutput[id].BoneWeight.y = weight;
					break;
				case 3:
					verticesOutput[id].BoneID[2] = i;
					verticesOutput[id].BoneWeight.z = weight;
					break;
				case 4:
					verticesOutput[id].BoneID[3] = i;
					verticesOutput[id].BoneWeight.w = weight;
					break;
				default:
					break;
			}
		}
	}

	
	//normalize weights to make all weights sum 1
	for (int i = 0; i < verticesOutput.size(); i++) 
	{
		XMFLOAT4& boneWeights = verticesOutput[i].BoneWeight;

		float totalWeight = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
		if (totalWeight > 0.0f) 
		{
			verticesOutput[i].BoneWeight = XMFLOAT4
			(
				boneWeights.x / totalWeight,
				boneWeights.y / totalWeight,
				boneWeights.z / totalWeight,
				boneWeights.w / totalWeight
			);
		}
	}
	

	//load indices
	Indices.reserve(mesh->mNumFaces);

	for (int i = 0; i < mesh->mNumFaces; i++) 
	{
		aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indicesOutput.push_back(face.mIndices[j]);
	}

	auto pd3dDevice = DXUTGetD3D11Device();

	D3D11_BUFFER_DESC vbd = {};
	vbd.ByteWidth = sizeof(SkinnedVertex::Mesh) * verticesOutput.size();
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData = {};
	vInitData.pSysMem = verticesOutput.data();

	ID3D11Buffer* vertexBuffer;
	pd3dDevice->CreateBuffer(&vbd, &vInitData, &vertexBuffer);
	g_ModelVertexBuffers.push_back(vertexBuffer);

	UINT indexCount = indicesOutput.size();
	g_ModelIndexCounts.push_back(indexCount);

	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(unsigned int) * indexCount;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData = {};
	iInitData.pSysMem = indicesOutput.data();

	ID3D11Buffer* indexBuffer;
	pd3dDevice->CreateBuffer(&ibd, &iInitData, &indexBuffer);
	g_ModelIndexBuffers.push_back(indexBuffer);

	// create bone hirerchy
	ExtractSkeleton(skeletonOutput, scene->mRootNode, boneInfo);
}

std::pair<UINT, float> SkinnedModel::getTimeFraction(std::vector<float>& times, float& dt)
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
void SkinnedModel::getPose(Animation& animation, Bone& skeletion, float dt, std::vector<XMFLOAT4X4>& output, XMFLOAT4X4& parentTransform, XMFLOAT4X4& globalInverseTransform)
{
	BoneTransformTrack& boneTransformTrack = animation.boneTransforms[skeletion.name];

	XMMATRIX scaleMatrix = XMMatrixIdentity();
	XMMATRIX positionMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();

	// calculate interpolated position
	std::pair <UINT, float> fp;
	fp = getTimeFraction(boneTransformTrack.positionTimestamps, dt);

	XMVECTOR position1 = XMLoadFloat3(&boneTransformTrack.positions[fp.first - 1]);
	XMVECTOR position2 = XMLoadFloat3(&boneTransformTrack.positions[fp.first]);
	XMVECTOR position = XMVectorLerp(position1, position2, fp.second);

	// calculate interpolated scale
	fp = getTimeFraction(boneTransformTrack.scaleTimestamps, dt);
	XMVECTOR scale1 = XMLoadFloat3(&boneTransformTrack.scales[fp.first - 1]);
	XMVECTOR scale2 = XMLoadFloat3(&boneTransformTrack.scales[fp.first]);
	XMVECTOR scale = XMVectorLerp(scale1, scale2, fp.second);

	//calculate interpolated rotation
	fp = getTimeFraction(boneTransformTrack.rotationTimestamps, dt);
	XMVECTOR rotation1 = XMLoadFloat4(&boneTransformTrack.rotations[fp.first - 1]);
	XMVECTOR rotation2 = XMLoadFloat4(&boneTransformTrack.rotations[fp.first]);
	XMVECTOR rotation = XMQuaternionSlerp(rotation1, rotation2, fp.second);

	positionMatrix = XMMatrixTranslation(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position));
	scaleMatrix = XMMatrixScaling(XMVectorGetX(scale), XMVectorGetY(scale), XMVectorGetZ(scale));
	rotationMatrix = XMMatrixRotationQuaternion(rotation);

	// XMMATRIX localTransform = scaleMatrix * rotationMatrix * positionMatrix;
	XMMATRIX localTransform = XMMatrixAffineTransformation(scale, XMVectorZero(), rotation, position);

	XMMATRIX globalTransform = localTransform * XMLoadFloat4x4(&parentTransform);
	
	XMMATRIX outputTmp = XMLoadFloat4x4(&skeletion.offset) * globalTransform * XMLoadFloat4x4(&globalInverseTransform);
	XMStoreFloat4x4(&output[skeletion.id], outputTmp);

	XMFLOAT4X4 globalTransformXM;
	XMStoreFloat4x4(&globalTransformXM, globalTransform);

	for (Bone& child : skeletion.children) 
	{
		getPose(animation, child, dt, output, globalTransformXM, globalInverseTransform);
	}
}

