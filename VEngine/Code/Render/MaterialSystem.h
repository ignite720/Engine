#pragma once
#include <unordered_map>
#include <UID.h>
#include <memory>
#include "System.h"

class Material;

struct MaterialSystem : System
{
	std::unordered_map<UID, std::unique_ptr<Material>> materials;

	MaterialSystem();
	Material* CreateMaterial(std::string textureFilename, std::string shaderFilename);
	Material* FindMaterial(UID uid);
	void CreateAllMaterials();
	void Cleanup();
};

extern MaterialSystem materialSystem;
