#include "vpch.h"
#include "ShaderItem.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ShaderSystem.h"

ShaderItem::ShaderItem(std::string shaderItemName_,
	std::wstring vertexShaderFilename_,
	std::wstring pixelShaderFilename_) :
	shaderItemName(shaderItemName_),
	pixelShaderFilename(pixelShaderFilename_),
	vertexShaderFilename(vertexShaderFilename_)
{
	vertexShader = ShaderSystem::FindVertexShader(vertexShaderFilename)->GetShader();
	pixelShader = ShaderSystem::FindPixelShader(pixelShaderFilename)->GetShader();
}
