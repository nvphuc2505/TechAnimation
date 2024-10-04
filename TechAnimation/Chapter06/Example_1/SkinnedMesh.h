#include <DXUT.h>
#include <string>
#include <unordered_map>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "Utils.h"
#include "Vertex.h"
using namespace DirectX;



// structure to hold bone tree (skeleton)
struct Bone 
{
	int id = 0; // position of the bone in final upload array
	std::string name = "";
	XMFLOAT4X4 offset;
	std::vector<Bone> children = {};
};


// sturction representing an animation track
struct BoneTransformTrack 
{
	std::vector<float> positionTimestamps = {};
	std::vector<float> rotationTimestamps = {};
	std::vector<float> scaleTimestamps = {};

	std::vector<XMFLOAT3> positions = {};
	std::vector<XMFLOAT4> rotations = {};
	std::vector<XMFLOAT3> scales = {};
};


// structure containing animation information
struct Animation 
{
	float duration = 0.0f;
	float ticksPerSecond = 1.0f;
	std::unordered_map<std::string, BoneTransformTrack> boneTransforms = {};
};





class SkinnedModel
{
public:
	SkinnedModel();
	SkinnedModel(const std::string& filename);
	~SkinnedModel();
	
	void LoadModel(const aiScene* scene, std::vector<SkinnedVertex::Mesh>& verticesOutput, std::vector<UINT>& indicesOutput, Bone& skeletonOutput, UINT& nBoneCount);

	void GetPose(Animation& animation, Bone& skeletion, float dt, std::vector<XMFLOAT4X4>& output, XMFLOAT4X4& parentTransform, XMFLOAT4X4& globalInverseTransform);
	void Render();

private:
	std::pair<UINT, float> GetTimeFraction(std::vector<float>& times, float& dt);
	void ReadVertices(const aiMesh* mesh);
	void ReadIndices(const aiMesh* mesh);
	bool ExtractSkeleton(Bone& boneOutput, aiNode* node, std::unordered_map<std::string, std::pair<int, XMFLOAT4X4>>& boneInfoTable);
	void ExtractAnimation(const aiScene* scene, Animation& animation);


	Bone mBoneInfo;
	UINT mBoneCount;
	Animation mAnimation;

	std::vector<SkinnedVertex::Mesh> mVertices;
	std::vector<UINT> mIndices;
	std::vector<UINT> mIndicesCount;
	std::vector<ID3D11Buffer*> mVertexBuffers;
	std::vector<ID3D11Buffer*> mIndexBuffers;
};