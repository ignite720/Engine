#pragma once
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <set>
#include <map>

using namespace DirectX;

struct Skeleton;
struct Joint;

struct BoneWeights
{
	inline static int MAX_WEIGHTS = 3;
	inline static int MAX_BONE_INDICES = 4;

	//@Todo: these really should be an array, but it's a pain in the ass to fill them up in the FBXImporter
	std::vector<double> weights;
	std::vector<int> boneIndex;
};

struct AnimFrame
{
	double time;
	XMFLOAT4 rot; //Quaternion
	XMFLOAT3 pos;
	XMFLOAT3 scale;
};

struct Animation
{
	//Maps joint index to AnimFrames
	std::map<int, std::vector<AnimFrame>> frames;

	float GetStartTime(int jointIndex)
	{
		return frames[jointIndex].front().time;
	}

	float GetEndTime(int jointIndex)
	{
		return frames[jointIndex].back().time;
	}

	void Interpolate(float t, Joint& joint, Skeleton* skeleton);
};

struct Joint
{
	XMMATRIX currentPose = XMMatrixIdentity();
	XMMATRIX inverseBindPose = XMMatrixIdentity();

	std::string name;

	int parentIndex = -1; //-1 is the root joint's index or if the bone doesn't have a parent
	int index = 0;
};

struct Skeleton
{
	std::vector<Joint> joints;

	//@Todo: think about makint Animation a pointer here,
	//animframes might be expensive to copy around if map resizes.
	std::map<std::string, Animation> animations;
	std::string currentAnimation;

	void AddJoint(Joint joint);
	int FindJointIndexByName(std::string name);
	void CreateAnimation(std::string animationName);
	Animation& GetCurrentAnimation();
};
