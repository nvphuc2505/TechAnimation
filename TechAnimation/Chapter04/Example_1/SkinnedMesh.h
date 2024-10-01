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
	SkinnedModel(const aiScene* scene);
	
	bool ExtractSkeleton(Bone& boneOutput, aiNode* node, std::unordered_map<std::string, std::pair<int, XMFLOAT4X4>>& boneInfoTable);
	void ExtractAnimation(const aiScene* scene, Animation& animation);
	void LoadModel(const aiScene* scene, aiMesh* mesh, std::vector<SkinnedVertex::Mesh>& verticesOutput, std::vector<UINT>& indicesOutput, Bone& skeletonOutput, UINT& nBoneCount);

	std::pair<UINT, float> getTimeFraction(std::vector<float>& times, float& dt);
	void getPose(Animation& animation, Bone& skeletion, float dt, std::vector<XMFLOAT4X4>& output, XMFLOAT4X4& parentTransform, XMFLOAT4X4& globalInverseTransform);

	Bone BoneInfo;
	UINT BoneCount;
	std::vector<SkinnedVertex::Mesh> Vertices;
	std::vector<UINT> Indices;
	std::vector<UINT> g_ModelIndexCounts;

	std::vector<ID3D11Buffer*> g_ModelVertexBuffers;
	std::vector<ID3D11Buffer*> g_ModelIndexBuffers;

	Animation mAnimation;

};