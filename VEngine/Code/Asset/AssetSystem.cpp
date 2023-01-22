#include "vpch.h"
#include "AssetSystem.h"
#include <cstdio>
#include <filesystem>
#include "FBXLoader.h"
#include "MeshAssetHeader.h"
#include "Profile.h"
#include "Log.h"
#include "FileSystem.h"
#include "Asset/AssetPaths.h"

std::map<std::string, MeshData> existingMeshData;

//@Todo: do something when importing all, remove the existing meshdata to match the meshes currently in world.
//That or make an offline process or a process when a file is added on a filewatcher's notice.
void AssetSystem::BuildAllVMeshDataFromFBXImport()
{
	FILE* file = nullptr;

	uint64_t numberOfMeshFilesBuilt = 0;

	auto startTime = Profile::QuickStart();

	std::unordered_map<std::string, MeshData> meshDataMap;

	//Import all FBX files
	for (const auto& entry : std::filesystem::directory_iterator("FBXFiles/"))
	{
		const std::string fbxFilename = entry.path().filename().string();
		meshDataMap.emplace(fbxFilename, MeshData());
		FBXLoader::ImportAsMesh(fbxFilename, meshDataMap.find(fbxFilename)->second);
	}

	for (auto& [filename, meshData] : meshDataMap)
	{
		MeshAssetHeader header;
		header.sourceMeshFormat = SourceMeshFormat::FBX;
		header.vertexCount = meshData.vertices.size();
		header.boneCount = meshData.skeleton.GetNumJoints();

		const std::string meshName = filename.substr(0, filename.find("."));
		const std::string meshFilePath = AssetBaseFolders::mesh + meshName + ".vmesh";

		fopen_s(&file, meshFilePath.c_str(), "wb");
		assert(file);

		assert(fwrite(&header, sizeof(MeshAssetHeader), 1, file));
		assert(fwrite(meshData.vertices.data(), sizeof(Vertex), meshData.vertices.size(), file));
		assert(fwrite(&meshData.boudingBox, sizeof(DirectX::BoundingBox), 1, file));

		auto& joints = meshData.skeleton.joints;
		fwrite(joints.data(), sizeof(Joint), joints.size(), file);

		fclose(file);

		numberOfMeshFilesBuilt++;
	}

	double elapsedTime = Profile::QuickEnd(startTime);

	Log("Mesh asset build complete.\n\tNum meshes built: %d\n\tTime taken: %f",
		numberOfMeshFilesBuilt, elapsedTime);
}

void AssetSystem::BuildAllAnimationFilesFromFBXImport()
{
	FILE* file = nullptr;

	uint64_t numberOfAnimationFilesBuilt = 0;

	auto startTime = Profile::QuickStart();

	std::unordered_map<std::string, Animation> animationMap;

	//Import all animated FBX files
	for (const auto& entry : std::filesystem::directory_iterator("AnimationFBXFiles/"))
	{
		const std::string fbxFilename = entry.path().filename().string();
		Animation anim = FBXLoader::ImportAsAnimation(fbxFilename);
		animationMap.emplace(fbxFilename, anim);
	}

	for (const auto& [filename, animation] : animationMap)
	{
		const std::string meshName = filename.substr(0, filename.find("."));
		const std::string meshFilePath = AssetBaseFolders::anim + meshName + ".vanim";

		fopen_s(&file, meshFilePath.c_str(), "wb");
		assert(file);

		AnimationAssetHeader header;
		strcpy_s(header.name, sizeof(char) * Animation::ANIM_NAME_MAX, animation.name);
		header.frameCount = animation.frames.size();
		assert(fwrite(&header, sizeof(AnimationAssetHeader), 1, file));

		for (auto& [jointIndex, animFrame] : animation.frames)
		{
			//Anim Frames
			assert(fwrite(&jointIndex, sizeof(int), 1, file));

			const size_t animFrameCount = animFrame.size();
			assert(fwrite(&animFrameCount, sizeof(size_t), 1, file));
			assert(fwrite(animFrame.data(), sizeof(AnimFrame) * animFrameCount, 1, file));
		}

		fclose(file);

		numberOfAnimationFilesBuilt++;
	}

	double elapsedTime = Profile::QuickEnd(startTime);

	Log("Animation asset build complete.\n\Num animations built: %d\n\tTime taken: %f",
		numberOfAnimationFilesBuilt, elapsedTime);
}

MeshDataProxy AssetSystem::ReadVMeshAssetFromFile(const std::string filename)
{
	std::string filepath = AssetBaseFolders::mesh + filename;

	FILE* file = nullptr;
	fopen_s(&file, filepath.c_str(), "rb");
	assert(file);

	MeshAssetHeader header;
	MeshData data;

	assert(fread(&header, sizeof(MeshAssetHeader), 1, file));

	data.vertices.resize(header.vertexCount);

	assert(fread(data.vertices.data(), sizeof(Vertex), header.vertexCount, file));
	assert(fread(&data.boudingBox, sizeof(DirectX::BoundingBox), 1, file));

	data.skeleton.joints.resize(header.boneCount);
	assert(fread(data.skeleton.joints.data(), sizeof(Joint), header.boneCount, file));

	fclose(file);
	
	existingMeshData.emplace(filename, data);
	auto& foundMeshData = existingMeshData.find(filename)->second;

	MeshDataProxy meshDataProxy;
	meshDataProxy.vertices = &foundMeshData.vertices;
	meshDataProxy.boundingBox = &foundMeshData.boudingBox;
	meshDataProxy.skeleton = &foundMeshData.skeleton;

	return meshDataProxy;
}

Animation AssetSystem::ReadVAnimAssetFromFile(const std::string filename)
{
	const std::string filepath = AssetBaseFolders::anim + filename;

	FILE* file = nullptr;
	fopen_s(&file, filepath.c_str(), "rb");
	assert(file);

	AnimationAssetHeader header;

	assert(fread(&header, sizeof(AnimationAssetHeader), 1, file));

	Animation anim(header.name);

	for (uint64_t i = 0; i < header.frameCount; i++)
	{
		int jointIndex = -2; //-2 is an invalid bone index
		assert(fread(&jointIndex, sizeof(int), 1, file));
		assert(jointIndex != -2);

		size_t animFrameCount = 0;
		assert(fread(&animFrameCount, sizeof(size_t), 1, file));

		std::vector<AnimFrame> animFrames;
		animFrames.reserve(animFrameCount);
		assert(fread(animFrames.data(), sizeof(AnimFrame) * animFrameCount, 1, file));

		anim.frames.emplace(jointIndex, animFrames);
	}

	fclose(file);

	return anim;
}

void AssetSystem::BuildAllGameplayMapFiles()
{
	for (auto const& dirEntry : std::filesystem::recursive_directory_iterator{ "WorldMaps" })
	{
		std::string file = dirEntry.path().filename().string();
		FileSystem::CreateGameplayWorldSave(file);
	}
}
