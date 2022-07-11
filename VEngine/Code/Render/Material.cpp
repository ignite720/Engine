#include "vpch.h"
#include "Material.h"
#include "RenderUtils.h"
#include "Renderer.h"
#include "ShaderSystem.h"
#include "TextureSystem.h"
#include "MaterialSystem.h"
#include "Texture2D.h"
#include "VString.h"
#include "WorldEditor.h"
#include "Actors/Actor.h"
#include "Components/MeshComponent.h"
#include "Log.h"

Material::Material(std::string textureFilename_, std::string shaderFilename_)
{
	textureData.filename = textureFilename_;
	shaderData.filename = shaderFilename_;

	//@Todo: I don't like this. There needs to be a way to check if something inherits from VEnum when serialising
	rastStateValue.Add(RastStates::solid);
	rastStateValue.Add(RastStates::noBackCull);
	rastStateValue.Add(RastStates::shadow);
	rastStateValue.Add(RastStates::wireframe);

	blendStateValue.Add(BlendStates::null);
	blendStateValue.Add(BlendStates::Default);
}

void Material::Create()
{
	texture = textureSystem.FindTexture2D(textureData.filename);
	sampler = RenderUtils::GetDefaultSampler();
	shader = shaderSystem.FindShader(VString::stows(shaderData.filename));
	rastState = Renderer::GetRastState(rastStateValue.GetValue());
	blendState = Renderer::GetBlendState(blendStateValue.GetValue());
}

void Material::Destroy()
{
	materialSystem.materials.erase(uid);
}

static void ReassignTexture(void* data)
{
	auto textureData = (TextureData*)data;

	Texture2D* swapTexture = textureSystem.FindTexture2D(textureData->filename, true);
	if (swapTexture == nullptr)
	{
		Log("%s wasn't found on texture change.", textureData->filename);
		return;
	}

	//@Todo: these 'get all mesh' calls aren't really right as each mesh component would
	//have its own properties, but it works ok for simple actors for now.
	auto meshes = worldEditor.pickedActor->GetComponentsOfType<MeshComponent>();
	for (auto mesh : meshes)
	{
		mesh->material->texture = swapTexture;
	}
}

static void ReassignRastState(void* data)
{
	auto rastState = (VEnum*)data;
	auto rastName = rastState->GetValue();
	RastState* foundRastState = Renderer::GetRastState(rastName);
	if (foundRastState == nullptr)
	{
		Log("[%s] not found on rast state change.", rastName.c_str());
		return;
	}

	if (worldEditor.pickedActor)
	{
		auto meshes = worldEditor.pickedActor->GetComponentsOfType<MeshComponent>();
		for (auto mesh : meshes)
		{
			mesh->material->rastState = foundRastState;
		}
	}
}

static void ReassignBlendState(void* data)
{
	auto blendState = (VEnum*)data;
	auto blendName = blendState->GetValue();
	BlendState* foundBlendState = Renderer::GetBlendState(blendName);
	if (foundBlendState == nullptr)
	{
		Log("[%s] not found on blend state change.", blendName.c_str());
		return;
	}

	if (worldEditor.pickedActor)
	{
		auto meshes = worldEditor.pickedActor->GetComponentsOfType<MeshComponent>();
		for (auto mesh : meshes)
		{
			mesh->material->blendState = foundBlendState;
		}
	}
}

static void ReassignShader(void* data)
{
	auto shaderData = (ShaderData*)data;
	ShaderItem* foundShader = shaderSystem.FindShader(VString::stows(shaderData->filename));
	if (foundShader == nullptr)
	{
		Log("%s not found on shader change.", shaderData->filename);
		return;
	}

	auto meshes = worldEditor.pickedActor->GetComponentsOfType<MeshComponent>();
	for (auto mesh : meshes)
	{
		mesh->material->shader = foundShader;
	}
}

Properties Material::GetProps()
{
	Properties props("Material");
	props.Add("Texture", &textureData).change = ReassignTexture;
	props.Add("Use Texture", &materialShaderData.useTexture);
	props.Add("Shader", &shaderData).change = ReassignShader;
	props.Add("Rast State", &rastStateValue).change = ReassignRastState;
	props.Add("Blend State", &blendStateValue).change = ReassignBlendState;
	props.Add("UvOffset", &materialShaderData.uvOffset);
	props.Add("UvOffsetSpeed", &uvOffsetSpeed);
	props.Add("UvScale", &materialShaderData.uvScale);
	props.Add("UvRotation", &materialShaderData.uvRotation);
	props.Add("UvRotationSpeed", &uvRotationSpeed);
	props.Add("Ambient", &materialShaderData.ambient);
	props.Add("Specular", &materialShaderData.specular);
	props.Add("Smoothness", &materialShaderData.smoothness);
	props.Add("Metallic", &materialShaderData.metallic);
	return props;
}
