#include "vpch.h"
#include "Renderer.h"
#include <dxgi1_6.h>
#include "RenderTypes.h"
#include "PipelineObjects.h"
#include <ScreenGrab.h>
#include <SHMath/DirectXSH.h>
#include <WinCodec.h> //For GUID_ContainerFormatJpeg
#include <filesystem>
#include <array>
#include "Debug.h"
#include "Core.h"
#include "Log.h"
#include "VMath.h"
#include "ShaderSystem.h"
#include "Camera.h"
#include "WorldEditor.h"
#include "Input.h"
#include "Material.h"
#include "Profile.h"
#include "RenderUtils.h"
#include "Texture2D.h"
#include "TextureSystem.h"
#include "ConstantBuffer.h"
#include "UI/UISystem.h"
#include "ShadowMap.h"
#include "Actors/Actor.h"
#include "Actors/DiffuseProbeMap.h"
#include "Actors/PostProcessInstance.h"
#include "Actors/DebugActors/DebugBox.h"
#include "Actors/DebugActors/DebugSphere.h"
#include "Actors/DebugActors/DebugIcoSphere.h"
#include "Actors/DebugActors/DebugCamera.h"
#include "Actors/DebugActors/DebugCone.h"
#include "Components/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxTriggerComponent.h"
#include "Components/InstanceMeshComponent.h"
#include "Components/Lights/DirectionalLightComponent.h"
#include "Components/Lights/PointLightComponent.h"
#include "Components/Lights/SpotLightComponent.h"
#include "Components/WidgetComponent.h"
#include "Editor/DebugMenu.h"
#include "Gameplay/GameInstance.h"
#include "Particle/ParticleEmitter.h"
#include "Particle/SpriteSheet.h"
#include "Particle/Polyboard.h"
#include "Physics/Raycast.h"
#include "Render/SpriteSystem.h"
#include "Render/VertexShader.h"
#include "Render/PixelShader.h"

void CreateFactory();
void CreateDevice();
void CreateSwapchain(HWND window);
void CreateRTVAndDSV();
void CreateInputLayout();
void CreateRasterizerStates();
void CreateBlendStates();
void CreateConstantBuffers();
void CreateLightProbeBuffers();
void CheckSupportedFeatures();
void RenderShadowPass();
void RenderMeshComponents();
void AnimateAndRenderSkeletalMeshes();
void RenderInstanceMeshComponents();
void RenderBounds();
void RenderCameraMeshes();
void RenderLightMeshes();
void RenderPolyboards();
void RenderSpriteSheets();
void RenderPostProcess();
void UpdateLights();
void MapBuffer(ID3D11Resource* resource, const void* src, size_t size);
void DrawMesh(MeshComponent* mesh);
void DrawMeshInstanced(InstanceMeshComponent* mesh);

void RenderDebugLines();

//Inner render functions to set shader resources
void SetNullRTV();
void SetShadowData();
void SetLightResources();
void SetShadowResources();
void SetMatricesFromMesh(MeshComponent* mesh);
void SetShaderMeshData(MeshComponent* mesh);
void SetRenderPipelineStates(MeshComponent* mesh);
void SetRenderPipelineStatesForShadows(MeshComponent* mesh);
void SetShaders(ShaderItem* shaderItem);
void SetRastState(std::string rastStateName);
void SetBlendState(std::string blendStateName);
void SetConstantBufferVertexPixel(uint32_t shaderRegister, ID3D11Buffer* constantBuffer);
void SetConstantBufferVertex(uint32_t shaderRegister, ID3D11Buffer* constantBuffer);
void SetConstantBufferPixel(uint32_t shaderRegister, ID3D11Buffer* constantBuffer);
void SetVertexBuffer(Buffer* vertexBuffer);
void SetIndexBuffer(Buffer* indexBuffer);
void SetSampler(uint32_t shaderRegister, Sampler* sampler);
void SetShaderResourcePixel(uint32_t shaderRegister, std::string textureName);
void SetShaderResourceFromMaterial(uint32_t shaderRegister, Material* material);

void CreatePostProcessRenderTarget();

float Renderer::frameTime;
bool Renderer::drawBoundingBoxes = false;
bool Renderer::drawTriggers = true;
bool Renderer::drawAllAsWireframe = false;

unsigned int Renderer::stride = sizeof(Vertex);
unsigned int Renderer::offset = 0;

DXGI_FORMAT indexBufferFormat = DXGI_FORMAT_R32_UINT;

ID3D11Texture2D* backBuffer;
ID3D11Texture2D* depthStencilBuffer;

static const int swapchainCount = 2;

ID3D11Device* device;
ID3D11DeviceContext* context;

ID3D11RenderTargetView* rtvs[swapchainCount];

ID3D11DepthStencilView* dsv;
ID3D11InputLayout* inputLayout;

//Rasterizer states
std::map<std::string, std::unique_ptr<RastState>> rastStateMap;
std::map<std::string, std::unique_ptr<BlendState>> blendStateMap;
ID3D11RasterizerState* rastStateSolid;
ID3D11RasterizerState* rastStateWireframe;
ID3D11RasterizerState* rastStateNoBackCull;
ID3D11RasterizerState* rastStateShadow;

//Blendstates
ID3D11BlendState* nullBlendState = nullptr;
ID3D11BlendState* blendStateAlphaToCoverage = nullptr;

//DXGI
IDXGISwapChain3* swapchain;
IDXGIFactory6* dxgiFactory;

//Constant buffers and data
ConstantBuffer<ShaderMatrices>* cbMatrices;
ConstantBuffer<MaterialShaderData>* cbMaterial;
ConstantBuffer<ShaderLights>* cbLights;
ConstantBuffer<ShaderTimeData>* cbTime;
ConstantBuffer<ShaderMeshData>* cbMeshData;
ConstantBuffer<ShaderSkinningData>* cbSkinningData;
ConstantBuffer<ShaderPostProcessData>* cbPostProcess;

//Viewport
D3D11_VIEWPORT viewport;

//Shadow maps
ShadowMap* shadowMap;

//Light probe buffers
ID3D11RenderTargetView* lightProbeRTVs[6]; //Cubemap
ID3D11ShaderResourceView* lightProbeSRV = nullptr;
ID3D11Texture2D* lightProbeTexture = nullptr;

//Post process
ID3D11Texture2D* postBuffer = nullptr;
ID3D11RenderTargetView* postRTV = nullptr;
ID3D11ShaderResourceView* postSRV = nullptr;

//Quality = 0 and Count = 1 are the 'default'
DXGI_SAMPLE_DESC sampleDesc;

const int shadowMapTextureResgiter = 1;
const int reflectionTextureResgiter = 2;
const int instanceSRVRegister = 3;
const int environmentMapTextureRegister = 4;
const int normalMapTexureRegister = 5;

const int lightProbeTextureWidth = 64;
const int lightProbeTextureHeight = 64;

ShaderMatrices shaderMatrices;
ShaderLights shaderLights;

//Debug object containers
std::vector<DirectX::BoundingOrientedBox> debugOrientedBoxesOnTimerToRender;
std::vector<Vertex> debugLines;
ID3D11Buffer* debugLinesBuffer;
static const uint64_t debugLinesBufferSize = 32 * 32 * sizeof(Vertex);

void Renderer::Init(void* window, int viewportWidth, int viewportHeight)
{
	viewport.Width = viewportWidth;
	viewport.Height = viewportHeight;
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	CreateFactory();
	CreateDevice();

	ShaderSystem::Init();

	shadowMap = new ShadowMap(device, 2048, 2048);

	CheckSupportedFeatures();

	CreateSwapchain((HWND)window);
	CreateRTVAndDSV();
	CreateInputLayout();
	CreateRasterizerStates();
	CreateBlendStates();
	CreateConstantBuffers();

	CreatePostProcessRenderTarget();

	RenderUtils::defaultSampler = RenderUtils::CreateSampler();

	SpriteSystem::Init();

	//debugLines.emplace_back(Vertex()); //dummy data so DirecX doesn't crash
	////@Todo: this craahses the program a lot (most dynamic buffers do) and not sure why.
	//debugLinesBuffer = RenderUtils::CreateDynamicBuffer(debugLinesBufferSize, D3D11_BIND_VERTEX_BUFFER, debugLines.data());
	//debugLines.clear();
}

void Renderer::Tick()
{
	//BOUNDING BOXES HOTKEY
	if (Input::GetKeyHeld(Keys::Ctrl))
	{
		if (Input::GetKeyDown(Keys::B))
		{
			drawBoundingBoxes = !drawBoundingBoxes;
		}
	}

	//RENDER TRIGGERS HOTKEY
	if (Input::GetKeyHeld(Keys::Ctrl))
	{
		if (Input::GetKeyDown(Keys::T))
		{
			drawTriggers = !drawTriggers;
		}
	}

	//DRAW ALL AS WIREFRAME HOTKEY
	if (Input::GetKeyUp(Keys::F2))
	{
		drawAllAsWireframe = !drawAllAsWireframe;
	}

	ScreenshotCapture();
}

void CreateFactory()
{
	IDXGIFactory* tempDxgiFactory = nullptr;
	HR(CreateDXGIFactory(IID_PPV_ARGS(&tempDxgiFactory)));
	HR(tempDxgiFactory->QueryInterface(&dxgiFactory));
	tempDxgiFactory->Release();
}

void CreateDevice()
{
	//BGRA support needed for DirectWrite and Direct2D
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
	D3D_FEATURE_LEVEL selectedFeatureLevel;

	//@Todo: this shit always causes problems. From HDR to graphics debugging, all sorts of issues.
	//For now, keep adapter CreateDevice() input as nullptr. Change on release.
	//IDXGIAdapter1* adapter = nullptr;
	//HR(dxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));

	HR(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
		featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &device,
		&selectedFeatureLevel, &context));

	RenderUtils::device = device;
	RenderUtils::context = context;
}

void CreateSwapchain(HWND window)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc = { (UINT)viewport.Width, (UINT)viewport.Height, {60, 1}, DXGI_FORMAT_R16G16B16A16_FLOAT };
	sd.Windowed = TRUE;
	sd.SampleDesc = sampleDesc;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = window;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.BufferCount = swapchainCount;

	IDXGISwapChain* tempSwapchain = nullptr;
	HR(dxgiFactory->CreateSwapChain(device, &sd, &tempSwapchain));
	HR(tempSwapchain->QueryInterface(&swapchain));
	tempSwapchain->Release();

	HR(swapchain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709));

	//Check for colour space (HDR, sRGB)
	IDXGIOutput* output = nullptr;
	HR(swapchain->GetContainingOutput(&output));
	IDXGIOutput6* output6 = nullptr;
	HR(output->QueryInterface<IDXGIOutput6>(&output6));
	DXGI_OUTPUT_DESC1 outputDesc = {};
	HR(output6->GetDesc1(&outputDesc));

	dxgiFactory->MakeWindowAssociation(window, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
}

void CreateRTVAndDSV()
{
	swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

	//Create Render target views
	for (int i = 0; i < swapchainCount; i++)
	{
		HR(device->CreateRenderTargetView(backBuffer, nullptr, &rtvs[i]));
	}

	//Create depth stencil view
	D3D11_TEXTURE2D_DESC dsDesc = {};
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.ArraySize = 1;
	dsDesc.MipLevels = 1;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.SampleDesc = sampleDesc;
	dsDesc.Width = viewport.Width;
	dsDesc.Height = viewport.Height;

	HR(device->CreateTexture2D(&dsDesc, nullptr, &depthStencilBuffer));
	assert(depthStencilBuffer);
	HR(device->CreateDepthStencilView(depthStencilBuffer, nullptr, &dsv));
}

void CreateInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(Vertex, boneIndices), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, weights), D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	VertexShader* shader = ShaderSystem::FindVertexShader(L"Default_vs.cso");
	
	HR(device->CreateInputLayout(inputDesc, _countof(inputDesc), shader->GetByteCodeData(), shader->GetByteCodeSize(), &inputLayout));
	context->IASetInputLayout(inputLayout);
}

void CreateRasterizerStates()
{
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.DepthClipEnable = TRUE;
	rastDesc.FrontCounterClockwise = FALSE;

	//SOLID
	{
		HR(device->CreateRasterizerState(&rastDesc, &rastStateSolid));

		rastStateMap.emplace(RastStates::solid, std::make_unique<RastState>(RastStates::solid, rastStateSolid));
	}

	//WIREFRAME
	{
		rastDesc.FillMode = D3D11_FILL_WIREFRAME;
		rastDesc.CullMode = D3D11_CULL_NONE;
		HR(device->CreateRasterizerState(&rastDesc, &rastStateWireframe));

		rastStateMap.emplace(RastStates::wireframe, std::make_unique<RastState>(RastStates::wireframe, rastStateWireframe));
	}

	//SOLID, NO BACK CULL
	{
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_SOLID;
		HR(device->CreateRasterizerState(&rastDesc, &rastStateNoBackCull));

		rastStateMap.emplace(RastStates::noBackCull, std::make_unique<RastState>(RastStates::noBackCull, rastStateNoBackCull));
	}

	//SHADOWS
	{
		rastDesc.CullMode = D3D11_CULL_BACK;
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.DepthBias = 100000;
		rastDesc.DepthBiasClamp = 0.0f;
		rastDesc.SlopeScaledDepthBias = 1.0f;
		HR(device->CreateRasterizerState(&rastDesc, &rastStateShadow));

		rastStateMap.emplace(RastStates::shadow, std::make_unique<RastState>(RastStates::shadow, rastStateShadow));
	}
}

void CreateBlendStates()
{
	//NULL BLEND STATE
	{
		D3D11_BLEND_DESC nullBlendDesc = {};
		nullBlendDesc.RenderTarget[0].BlendEnable = false;
		HR(device->CreateBlendState(&nullBlendDesc, &blendStateAlphaToCoverage));

		blendStateMap.emplace(BlendStates::null, std::make_unique<BlendState>(BlendStates::null, nullBlendState));
	}

	//DEFAULT BLEND STATE
	{
		D3D11_BLEND_DESC alphaToCoverageDesc = {};
		//MSAA has to be set for AlphaToCoverage to work.
		//alphaToCoverageDesc.AlphaToCoverageEnable = true;
		alphaToCoverageDesc.RenderTarget[0].BlendEnable = true;
		alphaToCoverageDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		alphaToCoverageDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		alphaToCoverageDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		alphaToCoverageDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		alphaToCoverageDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		alphaToCoverageDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		HR(device->CreateBlendState(&alphaToCoverageDesc, &blendStateAlphaToCoverage));

		blendStateMap.emplace(BlendStates::Default, std::make_unique<BlendState>(BlendStates::Default, blendStateAlphaToCoverage));
	}
}

void CreateConstantBuffers()
{
	//Registers
	const int cbMatrixRegister = 0;
	const int cbMaterialRegister = 1;
	const int cbSkinningRegister = 2;
	const int cbLightsRegister = 3;
	const int cbTimeRegister = 4;
	const int cbMeshDataRegister = 5;
	const int cbMeshLightMapRegister = 6;

	//Shader matrix constant buffer
	shaderMatrices.Create();

	cbMatrices = new ConstantBuffer<ShaderMatrices>(&shaderMatrices, cbMatrixRegister);
	assert(cbMatrices);

	//Material buffer
	MaterialShaderData materialShaderData = {};
	cbMaterial = new ConstantBuffer<MaterialShaderData>(&materialShaderData, cbMaterialRegister);
	assert(cbMaterial);

	//Lights buffer
	ShaderLights shaderLights = {};
	cbLights = new ConstantBuffer<ShaderLights>(&shaderLights, cbLightsRegister);
	assert(cbLights);

	//Time buffer
	ShaderTimeData timeData = {};
	cbTime = new ConstantBuffer<ShaderTimeData>(&timeData, cbTimeRegister);
	assert(cbTime);

	//Mesh data buffer
	ShaderMeshData meshData = {};
	cbMeshData = new ConstantBuffer<ShaderMeshData>(&meshData, cbMeshDataRegister);
	assert(cbMeshData);

	//Skinning data
	ShaderSkinningData skinningData = {};
	cbSkinningData = new ConstantBuffer<ShaderSkinningData>(&skinningData, cbSkinningRegister);
	assert(cbSkinningData);

	//Post process data
	ShaderPostProcessData postProcessData = {};
	cbPostProcess = new ConstantBuffer<ShaderPostProcessData>(&postProcessData, 0);
	assert(cbPostProcess);
}

void MapBuffer(ID3D11Resource* resource, const void* src, size_t size)
{
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HR(context->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	std::memcpy(mapped.pData, src, size);
	context->Unmap(resource, 0);
}

void SetNullRTV()
{
	ID3D11RenderTargetView* nullRTV = nullptr;
	context->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void SetShadowData()
{
	if (DirectionalLightComponent::system.GetNumComponents() > 0)
	{
		shaderMatrices.lightMVP = shadowMap->OutputMatrix();
		shaderMatrices.lightViewProj = shadowMap->GetLightViewMatrix() * shadowMap->GetLightPerspectiveMatrix();

		shaderLights.shadowsEnabled = true;
	}
	else
	{
		shaderLights.shadowsEnabled = false;
	}
}

void SetLightResources()
{
	cbLights->SetVSAndPS();
}

void DrawMesh(MeshComponent* mesh)
{
	context->Draw(mesh->meshDataProxy.vertices->size(), 0);
}

void DrawMeshInstanced(InstanceMeshComponent* mesh)
{
	context->DrawInstanced(mesh->meshDataProxy.vertices->size(), mesh->GetInstanceCount(), 0, 0);
}

//@Todo: can change the input assembly to only take in Line structs to the shader stage. Right now it's taking
//in entire verticies and might be too heavy when visualsing entire lightmap raycast networks.
void RenderDebugLines()
{
	MaterialShaderData materialShaderData{};
	materialShaderData.ambient = XMFLOAT4(1.0f, 0.0f, 0.f, 1.0f);
	cbMaterial->Map(&materialShaderData);
	cbMaterial->SetPS();
	SetShaders(ShaderItems::SolidColour);

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	HR(context->Map(debugLinesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, debugLines.data(), debugLinesBufferSize);
	context->Unmap(debugLinesBuffer, 0);

	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	context->IASetVertexBuffers(0, 1, &debugLinesBuffer, &Renderer::stride, &Renderer::offset);

	shaderMatrices.model = XMMatrixIdentity();
	shaderMatrices.MakeModelViewProjectionMatrix();
	cbMatrices->Map(&shaderMatrices);
	cbMatrices->SetVS();

	context->Draw(debugLines.size(), 0);
}

void CheckSupportedFeatures()
{
	//Threading check
	D3D11_FEATURE_DATA_THREADING threadFeature = {};
	HR(device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadFeature, sizeof(threadFeature)));

	//Swapchain's SwapEffect needs to be DXGI_SWAP_EFFECT_DISCARD for MSAA to work.
	const int sampleCount = 1;

	//MSAA check and set
	UINT msaaQualityLevel;
	HR(device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampleCount, &msaaQualityLevel));
	assert(msaaQualityLevel > 0);

	//Quality has to be one less than what CheckMultisampleQualityLevels() spits out for some reason
	sampleDesc.Quality = msaaQualityLevel - 1;
	sampleDesc.Count = sampleCount;
}

void RenderShadowPass()
{
	Profile::Start();

	if (!shaderLights.shadowsEnabled)
	{
		return;
	}

	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(inputLayout);

	shadowMap->BindDsvAndSetNullRenderTarget(context);

	for (auto& mesh : MeshComponent::system.GetComponents())
	{
		if (!mesh->castsShadow || !mesh->IsActive())
		{
			continue;
		}

		SetRenderPipelineStatesForShadows(mesh.get());

		//Set matrices
		shaderMatrices.model = mesh->GetWorldMatrix();
		shaderMatrices.MakeModelViewProjectionMatrix();
		shaderMatrices.MakeTextureMatrix(mesh->material);

		cbMatrices->Map(&shaderMatrices);
		cbMatrices->SetVS();

		//Set textures
		Material* mat = mesh->material;
		context->PSSetSamplers(0, 1, &mat->sampler->data);
		SetShaderResourceFromMaterial(0, mesh->material);

		//Draw
		context->Draw(mesh->meshDataProxy.vertices->size(), 0);
	}

	SetNullRTV();

	Profile::End();
}

void RenderSetup()
{
	context->RSSetViewports(1, &viewport);

	const float clearColour[4] = { 0.f, 0.f, 0.f, 1.f };
	UINT frameIndex = swapchain->GetCurrentBackBufferIndex();

	context->ClearRenderTargetView(rtvs[frameIndex], clearColour);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	context->OMSetRenderTargets(1, &rtvs[frameIndex], dsv);

	context->IASetInputLayout(inputLayout);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RenderPostProcessSetup()
{
	context->RSSetViewports(1, &viewport);

	constexpr float clearColour[4] = { 0.f, 0.f, 0.f, 1.f };
	UINT frameIndex = swapchain->GetCurrentBackBufferIndex();

	context->ClearRenderTargetView(postRTV, clearColour);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	context->OMSetRenderTargets(1, &postRTV, dsv);

	context->IASetInputLayout(inputLayout);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SetShadowResources()
{
	context->PSSetShaderResources(shadowMapTextureResgiter, 1, &shadowMap->depthMapSRV);
	context->PSSetSamplers(1, 1, &shadowMap->sampler);
}

void Renderer::Render()
{
	Profile::Start();

	shaderMatrices.view = activeCamera->GetViewMatrix();
	shaderMatrices.proj = activeCamera->GetProjectionMatrix();

	//Set time constant buffer
	ShaderTimeData timeData = {};
	timeData.deltaTime = Core::GetDeltaTime();
	timeData.timeSinceStartup = Core::timeSinceStartup;

	cbTime->Map(&timeData);
	cbTime->SetVS();

	SetShadowData();
	RenderShadowPass();

	if (PostProcessInstance::system.GetNumActors() > 0 
		&& PostProcessInstance::system.GetFirstActor()->IsActive())
	{
		RenderPostProcessSetup();
	}
	else
	{
		RenderSetup();
	}

	UpdateLights();

	RenderMeshComponents();
	AnimateAndRenderSkeletalMeshes();
	RenderInstanceMeshComponents();
	RenderPolyboards();
	RenderSpriteSheets();
	RenderBounds();
	RenderLightMeshes();
	RenderCameraMeshes();
	//RenderDebugLines();
	RenderPostProcess();

	Profile::End();
}

void SetMatricesFromMesh(MeshComponent* mesh)
{
	shaderMatrices.model = mesh->GetWorldMatrix();
	shaderMatrices.MakeModelViewProjectionMatrix();
	shaderMatrices.MakeTextureMatrix(mesh->material);

	cbMatrices->Map(&shaderMatrices);
	cbMatrices->SetVS();
}

void SetShaderMeshData(MeshComponent* mesh)
{
	ShaderMeshData meshData = {};
	meshData.position = mesh->GetPosition();

	//@Todo: light probe data should have its own constant buffer, for now in testing, it's part of ShaderMeshData
	//Set light probe resources
	if (!DiffuseProbeMap::system.GetActors().empty())
	{
		context->PSSetShaderResources(environmentMapTextureRegister, 1, &lightProbeSRV);

		ProbeData probeData = DiffuseProbeMap::system.GetFirstActor()->FindClosestProbe(mesh->GetWorldPositionV());
		memcpy(meshData.SH, probeData.SH, sizeof(XMFLOAT4) * 9);
	}

	cbMeshData->Map(&meshData);
	cbMeshData->SetVSAndPS();
}

void RenderMeshComponents()
{
	Profile::Start();

	//Shader Resources
	SetShadowResources();
	SetLightResources();

	for (auto mesh : MeshComponent::SortMeshComponentsByDistanceToCamera())
	{
		if (!mesh->IsActive()) { continue; }

		SetRenderPipelineStates(mesh);

		//Constant buffer data
		SetMatricesFromMesh(mesh);
		SetShaderMeshData(mesh);

		DrawMesh(mesh);
	}	

	//Set to null to remove warnings
	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->PSSetShaderResources(shadowMapTextureResgiter, 1, &nullSRV);
	context->PSSetShaderResources(reflectionTextureResgiter, 1, &nullSRV);

	Profile::End();
}

void Renderer::RenderLightProbeViews()
{
	auto startTime = Profile::QuickStart();

	auto diffuseProbeMap = DiffuseProbeMap::system.GetFirstActor();
	if (diffuseProbeMap == nullptr)
	{
		Log("No diffuse probe map in level to render probes from.");
		return;
	}

	Log("Diffuse light probe map bake started...");

	const int previousWiewportWidth = viewport.Width;
	const int previousWiewportHeight = viewport.Height;
	ResizeSwapchain(lightProbeTextureWidth, lightProbeTextureHeight);

	//Directions match with D3D11_TEXTURECUBE_FACE
	XMVECTOR faces[6] =
	{
		XMVectorSet(1.f, 0.f, 0.f, 0.f), //+X
		XMVectorSet(-1.f, 0.f, 0.f, 0.f), //-X
		XMVectorSet(0.f, 1.f, 0.f, 0.f), //+Y
		XMVectorSet(0.f, -1.f, 0.f, 0.f), //-Y
		XMVectorSet(0.f, 0.f, 1.f, 0.f), //+Z
		XMVectorSet(0.f, 0.f, -1.f, 0.f), //-Z
	};

	CreateLightProbeBuffers();

	diffuseProbeMap->probeData.clear();

	int probeIndex = 0;

	//Disable instance meshes so the cubemap renders don't include them.
	diffuseProbeMap->instanceMeshComponent->SetActive(false);

	for (auto& instanceData : diffuseProbeMap->instanceMeshComponent->instanceData)
	{
		XMMATRIX& probeMatrix = instanceData.world;

		for (int i = 0; i < 6; i++)
		{
			context->RSSetViewports(1, &viewport);
			constexpr float clearColour[4] = { 0.f, 0.f, 0.f, 0.f };
			context->ClearRenderTargetView(lightProbeRTVs[i], clearColour);
			context->IASetInputLayout(inputLayout);
			context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			context->OMSetRenderTargets(1, &lightProbeRTVs[i], nullptr);

			context->RSSetState(rastStateMap[RastStates::solid]->data);

			UpdateLights();

			//Set lights buffer
			cbLights->SetPS();

			//Set shadow resources (not now for lightprobes)
			//context->PSSetShaderResources(shadowMapTextureResgiter, 1, &shadowMap->depthMapSRV);
			//context->PSSetSamplers(1, 1, &shadowMap->sampler);

			ShaderItem* lightProbeShader = ShaderItems::Default;

			for (auto& mesh : MeshComponent::system.GetComponents())
			{
				if (!mesh->IsActive()) { continue; }

				Material* material = mesh->material;

				const FLOAT blendState[4] = { 0.f };
				context->OMSetBlendState(nullptr, blendState, 0xFFFFFFFF);

				context->VSSetShader(lightProbeShader->GetVertexShader(), nullptr, 0);
				context->PSSetShader(lightProbeShader->GetPixelShader(), nullptr, 0);

				context->PSSetSamplers(0, 1, &material->sampler->data);

				SetShaderResourceFromMaterial(0, material);

				context->IASetVertexBuffers(0, 1, &mesh->pso.vertexBuffer->data, &stride, &offset);

				cbMaterial->Map(&material->materialShaderData);
				cbMaterial->SetPS();

				//Set matrices
				shaderMatrices.view = XMMatrixLookAtLH(probeMatrix.r[3],
					probeMatrix.r[3] + faces[i], VMath::GlobalUpVector());
				shaderMatrices.model = mesh->GetWorldMatrix();
				shaderMatrices.MakeModelViewProjectionMatrix();
				shaderMatrices.MakeTextureMatrix(mesh->material);

				cbMatrices->Map(&shaderMatrices);
				cbMatrices->SetVS();

				//Set mesh data to shader
				ShaderMeshData meshData = {};
				meshData.position = mesh->GetPosition();
				cbMeshData->Map(&meshData);
				cbMeshData->SetVSAndPS();

				//Draw
				context->Draw(mesh->meshDataProxy.vertices->size(), 0);
			}

			//Remove lightprobe RTV
			SetNullRTV();
		}

		//Remember that there are 9 coefficients with 3rd order SH per channel
		float SH_R[9] = {}, SH_G[9] = {}, SH_B[9] = {};
		HR(DirectX::SHProjectCubeMap(context, 3, lightProbeTexture, SH_R, SH_G, SH_B));

		XMFLOAT4 coefs[9] = {};
		for (int co_index = 0; co_index < 9; co_index++)
		{
			coefs[co_index] = XMFLOAT4(SH_R[co_index], SH_G[co_index], SH_B[co_index], 1.0f);
		}

		ProbeData pd;
		pd.index = probeIndex;
		memcpy(pd.SH, coefs, sizeof(XMFLOAT4) * 9);
		XMStoreFloat3(&pd.position, probeMatrix.r[3]);
		diffuseProbeMap->probeData.emplace_back(pd);

		probeIndex++;

		diffuseProbeMap->SetProbeVisualColourFromIrradiance(pd, instanceData);
	}

	ResizeSwapchain(previousWiewportWidth, previousWiewportHeight);

	//Set main RTV and DSV back on
	RenderSetup();

	diffuseProbeMap->WriteProbeDataToFile();

	diffuseProbeMap->instanceMeshComponent->SetActive(true);

	double endTime = Profile::QuickEnd(startTime);
	Log("Light probe bake took [%f]sec.", endTime);
}

void RenderInstanceMeshComponents()
{
	//@Todo: shadows for instancemeshes (might not even need it since Grid nodes are the only things rendererd that way)
	//@Todo: animated instance meshes

	Profile::Start();

	//Set matrices (Instance mesh model matrices placed in a structured buffer)
	shaderMatrices.model = XMMatrixIdentity();
	shaderMatrices.MakeModelViewProjectionMatrix();

	cbMatrices->Map(&shaderMatrices);
	cbMatrices->SetVS();

	for (auto& instanceMesh : InstanceMeshComponent::system.GetComponents())
	{
		if (!instanceMesh->IsActive()) continue;

		SetRenderPipelineStates(instanceMesh.get());

		//@Todo: clean this up in InstanceMeshComponent, can't every instance mesh as transparent
		SetBlendState(BlendStates::Default);

		//Update texture matrix
		shaderMatrices.MakeTextureMatrix(instanceMesh->material);
		cbMatrices->Map(&shaderMatrices);
		cbMatrices->SetVS();

		//Update instance data and set SRV
		MapBuffer(instanceMesh->structuredBuffer, instanceMesh->instanceData.data(), sizeof(InstanceData) * instanceMesh->instanceData.size());
		context->VSSetShaderResources(instanceSRVRegister, 1, &instanceMesh->srv);
		context->PSSetShaderResources(instanceSRVRegister, 1, &instanceMesh->srv);

		//Set lights buffer
		cbLights->SetPS();

		DrawMeshInstanced(instanceMesh.get());
	}

	Profile::End();
}

void RenderBounds()
{
	static DebugBox debugBox;

	MaterialShaderData materialShaderData = {};

	if (Renderer::drawBoundingBoxes)
	{
		SetRastState(RastStates::wireframe);
		SetShaders(ShaderItems::SolidColour);

		//@Todo: there's a weird error here where if you create enough lights in the map (about 40),
		//the debug mesh actors will crash here. Tried putting the Debug Actors as global pointers
		//instead of being static, but then Direct2D swapchain/rendertarget errors would happen.
		//Feels like it might be the GPU doing some funny memory thing.
		SetVertexBuffer(debugBox.boxMesh->GetVertexBuffer());

		//Set debug wireframe material colour
		materialShaderData.ambient = XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f);
		cbMaterial->Map(&materialShaderData);
		cbMaterial->SetPS();

		for (auto& mesh : MeshComponent::system.GetComponents())
		{
			DirectX::BoundingOrientedBox boundingBox = mesh->boundingBox;

			XMFLOAT3 extents = XMFLOAT3(boundingBox.Extents.x, boundingBox.Extents.y,
				boundingBox.Extents.z);

			XMVECTOR center = mesh->GetWorldPositionV() + XMLoadFloat3(&boundingBox.Center);
			XMVECTOR scale = mesh->GetScaleV() * XMLoadFloat3(&extents);

			XMMATRIX boundsMatrix = XMMatrixAffineTransformation(scale,
				XMVectorSet(0.f, 0.f, 0.f, 1.f),
				mesh->GetRotationV(),
				center);

			shaderMatrices.model = boundsMatrix;

			//Set bouding box scale just slightly more than the component to avoid mesh overlap
			shaderMatrices.model.r[0].m128_f32[0] += 0.01f;
			shaderMatrices.model.r[1].m128_f32[1] += 0.01f;
			shaderMatrices.model.r[2].m128_f32[2] += 0.01f;

			shaderMatrices.MakeModelViewProjectionMatrix();
			cbMatrices->Map(&shaderMatrices);
			cbMatrices->SetVS();

			DrawMesh(debugBox.boxMesh);
		}

		for (auto& box : debugOrientedBoxesOnTimerToRender)
		{
			XMFLOAT3 extents = XMFLOAT3(box.Extents.x * 2.f, box.Extents.y * 2.f, box.Extents.z * 2.f);

			XMVECTOR center = XMLoadFloat3(&box.Center);
			XMVECTOR scale = XMLoadFloat3(&extents);
			XMVECTOR rotation = XMLoadFloat4(&box.Orientation);

			XMMATRIX boundsMatrix = XMMatrixAffineTransformation(scale,
				XMVectorSet(0.f, 0.f, 0.f, 1.f),
				rotation,
				center);

			shaderMatrices.model = boundsMatrix;

			shaderMatrices.MakeModelViewProjectionMatrix();
			cbMatrices->Map(&shaderMatrices);
			cbMatrices->SetVS();

			DrawMesh(debugBox.boxMesh);
		}
	}

	//DRAW TRIGGER BOUNDS
	if(Renderer::drawTriggers)
	{
		SetRastState(RastStates::wireframe);
		SetShaders(ShaderItems::SolidColour);

		SetVertexBuffer(debugBox.boxMesh->GetVertexBuffer());

		for (auto& boxTrigger : BoxTriggerComponent::system.GetComponents())
		{
			shaderMatrices.model = boxTrigger->GetWorldMatrix();

			//Set to * 2.f because of extents
			shaderMatrices.model.r[0].m128_f32[0] *= boxTrigger->boundingBox.Extents.x * 2.f;
			shaderMatrices.model.r[1].m128_f32[1] *= boxTrigger->boundingBox.Extents.y * 2.f;
			shaderMatrices.model.r[2].m128_f32[2] *= boxTrigger->boundingBox.Extents.z * 2.f;

			shaderMatrices.model.r[3] += XMLoadFloat3(&boxTrigger->boundingBox.Center);

			shaderMatrices.MakeModelViewProjectionMatrix();
			cbMatrices->Map(&shaderMatrices);
			cbMatrices->SetVS();

			//Set trigger wireframe material colour
			materialShaderData.ambient = boxTrigger->renderWireframeColour;
			cbMaterial->Map(&materialShaderData);
			cbMaterial->SetPS();

			DrawMesh(debugBox.boxMesh);
		}
	}
}

void CreateLightProbeBuffers()
{
	//Texture
	if (lightProbeTexture) lightProbeTexture->Release();

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = lightProbeTextureWidth;
	texDesc.Height = lightProbeTextureWidth;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HR(device->CreateTexture2D(&texDesc, 0, &lightProbeTexture));
	assert(lightProbeTexture);

	//SRV
	if (lightProbeSRV) lightProbeSRV->Release();

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	HR(device->CreateShaderResourceView(lightProbeTexture, &srvDesc, &lightProbeSRV));

	//RTVs
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.ArraySize = 1;

	for (int i = 0; i < 6; i++)
	{
		if (lightProbeRTVs[i]) lightProbeRTVs[i]->Release();
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		HR(device->CreateRenderTargetView(lightProbeTexture, &rtvDesc, &lightProbeRTVs[i]));
	}
}

void RenderCameraMeshes()
{
	static DebugCamera debugCamera;

	if (Core::gameplayOn) return;

	MaterialShaderData materialShaderData = {};

	SetRastState(RastStates::wireframe);
	SetShaders(ShaderItems::SolidColour);
	SetVertexBuffer(debugCamera.mesh->GetVertexBuffer());

	materialShaderData.ambient = XMFLOAT4(1.0f, 0.0f, 0.f, 1.0f); //Make cameras red
	cbMaterial->Map(&materialShaderData);
	cbMaterial->SetPS();

	for (auto& camera : CameraComponent::system.GetComponents())
	{
		shaderMatrices.model = camera->GetWorldMatrix();
		shaderMatrices.MakeModelViewProjectionMatrix();
		cbMatrices->Map(&shaderMatrices);
		cbMatrices->SetVS();

		DrawMesh(debugCamera.mesh);
	}
}

void RenderLightMeshes()
{
	static DebugSphere debugSphere;
	static DebugIcoSphere debugIcoSphere;
	static DebugCone debugCone;

	if (Core::gameplayOn) return;

	SetRastState(RastStates::wireframe);
	SetShaders(ShaderItems::SolidColour);

	//Set debug sphere wireframe material colour
	MaterialShaderData materialShaderData = {};
	materialShaderData.ambient = XMFLOAT4(1.f, 1.f, 0.f, 1.0f);
	cbMaterial->Map(&materialShaderData);
	cbMaterial->SetPS();

	//DIRECTIONAL LIGHTS
	SetVertexBuffer(debugSphere.sphereMesh->GetVertexBuffer());

	for (auto& directionalLight : DirectionalLightComponent::system.GetComponents())
	{
		shaderMatrices.model = directionalLight->GetWorldMatrix();
		shaderMatrices.MakeModelViewProjectionMatrix();
		cbMatrices->Map(&shaderMatrices);
		cbMatrices->SetVS();

		DrawMesh(debugSphere.sphereMesh);
	}

	//POINT LIGHTS
	SetVertexBuffer(debugIcoSphere.mesh->GetVertexBuffer());

	for (auto& pointLight : PointLightComponent::system.GetComponents())
	{
		shaderMatrices.model = pointLight->GetWorldMatrix();
		shaderMatrices.MakeModelViewProjectionMatrix();
		cbMatrices->Map(&shaderMatrices);
		cbMatrices->SetVS();

		DrawMesh(debugIcoSphere.mesh);
	}

	//SPOT LIGHTS
	SetVertexBuffer(debugCone.mesh->GetVertexBuffer());

	for (auto& spotLight : SpotLightComponent::system.GetComponents())
	{
		shaderMatrices.model = spotLight->GetWorldMatrix();
		shaderMatrices.MakeModelViewProjectionMatrix();
		cbMatrices->Map(&shaderMatrices);
		cbMatrices->SetVS();

		DrawMesh(debugCone.mesh);
	}
}

void RenderPolyboards()
{
	Profile::Start();

	shaderMatrices.texMatrix = XMMatrixIdentity();
	shaderMatrices.model = XMMatrixIdentity();
	shaderMatrices.MakeModelViewProjectionMatrix();

	cbMatrices->Map(&shaderMatrices);
	cbMatrices->SetVS();

	SetBlendState(BlendStates::Default);
	SetRastState(RastStates::noBackCull);
	SetShaders(ShaderItems::DefaultClip);

	for (auto polyboard : World::GetAllComponentsOfType<Polyboard>())
	{
		polyboard->CalcVertices();

		context->PSSetSamplers(0, 1, &RenderUtils::GetDefaultSampler()->data);

		SetShaderResourcePixel(0, polyboard->textureData.filename);

		//VERTEX MAP
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource = {};
			HR(context->Map(polyboard->vertexBuffer->data, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			memcpy(mappedResource.pData, polyboard->vertices.data(), sizeof(Vertex) * polyboard->vertices.size());
			context->Unmap(polyboard->vertexBuffer->data, 0);
		}

		//INDEX MAP
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource = {};
			HR(context->Map(polyboard->indexBuffer->data, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			memcpy(mappedResource.pData, polyboard->indices.data(), sizeof(MeshData::indexDataType) * polyboard->indices.size());
			context->Unmap(polyboard->indexBuffer->data, 0);
		}

		SetVertexBuffer(polyboard->vertexBuffer);
		SetIndexBuffer(polyboard->indexBuffer);

		context->DrawIndexed(polyboard->indices.size(), 0, 0);
	}

	Profile::End();
}

void RenderSpriteSheets()
{
	Profile::Start();

	for (auto& spriteSheet : SpriteSheet::system.GetComponents())
	{
		SetRastState(RastStates::noBackCull);
		SetShaders(ShaderItems::DefaultClip);

		SetSampler(0, RenderUtils::GetDefaultSampler());
		SetShaderResourcePixel(0, spriteSheet->textureData.filename);

		spriteSheet->UpdateSprite();

		SpriteSystem::BuildSpriteQuadForSpriteSheetRendering(spriteSheet->sprite);
		SpriteSystem::UpdateAndSetSpriteBuffers(context);

		VMath::RotateTowardsCamera(spriteSheet->transform);

		shaderMatrices.model = spriteSheet->GetWorldMatrix();
		shaderMatrices.MakeModelViewProjectionMatrix();

		cbMatrices->Map(&shaderMatrices);
		cbMatrices->SetVS();

		context->DrawIndexed(6, 0, 0);
	}

	Profile::End();
}

void AnimateAndRenderSkeletalMeshes()
{
	Profile::Start();

	SetShadowResources();
	SetLightResources();

	for (auto& skeletalMesh : SkeletalMeshComponent::system.GetComponents()) //@Todo: figure out transparency sort
	{
		//Render
		if (!skeletalMesh->IsActive()) { continue; }

		//@Todo: shader set()s here are redundant with animation shaders below.
		SetRenderPipelineStates(skeletalMesh.get());

		SetMatricesFromMesh(skeletalMesh.get());
		SetShaderMeshData(skeletalMesh.get());

		//Animate
		Skeleton& skeleton = skeletalMesh->GetSkeleton();

		if (!skeletalMesh->GetCurrentAnimationName().empty())
		{
			if (!skeleton.joints.empty())
			{
				int skinningDataIndex = 0;
				ShaderSkinningData skinningData = {};

				//Set shader for skeletal animation
				ShaderItem* shaderItem = ShaderItems::Animation;
				context->VSSetShader(shaderItem->GetVertexShader(), nullptr, 0);
				context->PSSetShader(shaderItem->GetPixelShader(), nullptr, 0);

				Animation& anim = skeleton.GetCurrentAnimation();
				if (!anim.frames.empty())
				{
					skeletalMesh->IncrementAnimationTime(Core::GetDeltaTime());

					//Move through and animate all joints on skeleton
					for (Joint& joint : skeleton.joints)
					{
						if (skeletalMesh->GetCurrentAnimationTime() >= anim.GetEndTime(joint.index))
						{
							skeletalMesh->ResetAnimationTime();
						}

						//Blend testing
						if (!skeletalMesh->GetNextAnimationName().empty())
						{
							auto nextAnimIt = skeleton.animations.find(skeletalMesh->GetNextAnimationName());
							if (nextAnimIt != skeleton.animations.end())
							{
								anim.Interpolate(skeletalMesh->GetCurrentAnimationTime(), joint, &skeleton, &nextAnimIt->second, 0.5f);
							}
						}
						else
						{
							anim.Interpolate(skeletalMesh->GetCurrentAnimationTime(), joint, &skeleton, nullptr, 0.f);
						}


						skinningData.skinningMatrices[skinningDataIndex] = joint.currentPose;
						skinningDataIndex++;
						assert(skinningDataIndex < ShaderSkinningData::MAX_SKINNING_DATA);
					}
				}

				//Update skinning constant buffers
				if (skinningDataIndex > 0)
				{
					cbSkinningData->Map(&skinningData);
					cbSkinningData->SetVS();
				}
			}
		}

		DrawMesh(skeletalMesh.get());
	}

	//Set to null to remove warnings
	ID3D11ShaderResourceView* nullSRV = nullptr;
	context->PSSetShaderResources(shadowMapTextureResgiter, 1, &nullSRV);
	context->PSSetShaderResources(reflectionTextureResgiter, 1, &nullSRV);

	Profile::End();
}

void Renderer::RenderParticleEmitters()
{
	Profile::Start();

	//Only need to build sprite quad once for in-world rendering
	SpriteSystem::BuildSpriteQuadForParticleRendering();

	shaderMatrices.view = activeCamera->GetViewMatrix();
	shaderMatrices.proj = activeCamera->GetProjectionMatrix();
	shaderMatrices.texMatrix = XMMatrixIdentity();

	for (auto& emitter : ParticleEmitter::system.GetComponents())
	{
		if (!emitter->IsActive())
		{
			continue;
		}

		if (drawAllAsWireframe)
		{
			context->RSSetState(rastStateWireframe);
		}
		else
		{
			context->RSSetState(rastStateMap["nobackcull"]->data);
		}

		SetBlendState(BlendStates::Default);

		SetShaders(emitter->material->shader);

		context->PSSetSamplers(0, 1, &RenderUtils::GetDefaultSampler()->data);

		MaterialShaderData materialShaderData;
		materialShaderData = emitter->material->materialShaderData;
		cbMaterial->Map(&materialShaderData);
		cbMaterial->SetPS();

		//Set texture from emitter for every particle
		SetShaderResourcePixel(0, emitter->material->textureData.filename);

		SpriteSystem::UpdateAndSetSpriteBuffers(context);

		for (auto& particle : emitter->particles)
		{
			//Add rotation to particle (keep in mind that rotate speed needs to match angle's +/- value)
			particle.angle += particle.rotateSpeed * Core::GetDeltaTime();

			auto particleRotation = 
				VMath::LookAtRotation(activeCamera->GetWorldPositionV(), XMLoadFloat3(&particle.transform.position));
			XMStoreFloat4(&particle.transform.rotation, particleRotation);

			shaderMatrices.model = particle.transform.GetAffine();
			shaderMatrices.MakeModelViewProjectionMatrix();

			cbMatrices->Map(&shaderMatrices);
			cbMatrices->SetVS();

			//Note: Apparently using DrawInstanced() degrades performance when the vertex count it really low
			//and DrawIndexed is actually faster.
			context->DrawIndexed(6, 0, 0);
		}
	}

	Profile::End();
}

void Renderer::RenderSpritesInScreenSpace()
{
	Profile::Start();

	for (const auto& sprite : SpriteSystem::GetScreenSprites())
	{
		SetRastState(RastStates::solid);
		SetShaders(ShaderItems::UI);
		SetSampler(0, RenderUtils::GetDefaultSampler());
		SetShaderResourcePixel(0, sprite.textureFilename);

		SpriteSystem::BuildSpriteQuadForViewportRendering(sprite);
		SpriteSystem::UpdateAndSetSpriteBuffers(context);

		shaderMatrices.model = XMMatrixIdentity();
		shaderMatrices.MakeModelViewProjectionMatrix();
		cbMatrices->Map(&shaderMatrices);
		cbMatrices->SetVS();

		context->DrawIndexed(6, 0, 0);
	}

	Profile::End();
}

//Loops over every light component and moves their data into the lights constant buffer
void UpdateLights()
{
	Profile::Start();

	int shaderLightsIndex = 0;

	//Directional lights
	for (auto& light : DirectionalLightComponent::system.GetComponents())
	{
		if (!light->IsActive()) continue;

		Light lightData = light->lightData;
		XMFLOAT3 forwardVector = light->GetForwardVector();
		lightData.direction = XMFLOAT4(forwardVector.x, forwardVector.y, forwardVector.z, 0.f);

		shaderLights.lights[shaderLightsIndex] = lightData;
		shaderLightsIndex++;
	}

	//Point lights
	for (auto& light : PointLightComponent::system.GetComponents())
	{
		if (!light->IsActive()) continue;

		XMStoreFloat4(&light->lightData.position, light->GetWorldPositionV());

		shaderLights.lights[shaderLightsIndex] = light->lightData;
		shaderLightsIndex++;
	}
	
	//Spot lights
	for (auto& light : SpotLightComponent::system.GetComponents())
	{
		if (!light->IsActive()) continue;

		XMStoreFloat4(&light->lightData.position, light->GetWorldPositionV());

		XMFLOAT3 forwardVector = light->GetForwardVector();
		light->lightData.direction = XMFLOAT4(forwardVector.x, forwardVector.y, forwardVector.z, 0.f);

		shaderLights.lights[shaderLightsIndex] = light->lightData;
		shaderLightsIndex++;
	}

	shaderLights.numLights = shaderLightsIndex;

	XMStoreFloat4(&shaderLights.eyePosition, activeCamera->transform.world.r[3]);

	cbLights->Map(&shaderLights);
	cbLights->SetVSAndPS();

	Profile::End();
}

void Renderer::Present()
{
	HR(swapchain->Present(1, 0));
}

void* Renderer::GetSwapchain()
{
	return swapchain;
}

float Renderer::GetAspectRatio()
{
	return viewport.Width / viewport.Height;
}

float Renderer::GetViewportWidth()
{
	return viewport.Width;
}

float Renderer::GetViewportHeight()
{
	return viewport.Height;
}

void Renderer::SetViewportWidthHeight(float width, float height)
{
	viewport.Width = width;
	viewport.Height = height;
}

void Renderer::ResizeSwapchain(int newWidth, int newHeight)
{
	if (swapchain == nullptr) return;

	context->OMSetRenderTargets(0, 0, 0);

	// Release all outstanding references to the swap chain's buffers.
	for (int rtvIndex = 0; rtvIndex < swapchainCount; rtvIndex++)
	{
		rtvs[rtvIndex]->Release();
	}

	dsv->Release();

	UISystem::Cleanup();

	backBuffer->Release();
	HR(swapchain->ResizeBuffers(swapchainCount, newWidth, newHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	viewport.Width = newWidth;
	viewport.Height = newHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	context->RSSetViewports(1, &viewport);

	CreateRTVAndDSV();

	CreatePostProcessRenderTarget();

	UISystem::Init((void*)swapchain);

	shaderMatrices.Create();
}

void Renderer::ScreenshotCapture()
{
	if (Input::GetKeyUp(Keys::F8))
	{
		//Clear previous notification so nothing appears in the screenshot.
		debugMenu.debugNotifications.clear();

		ID3D11Texture2D* backBuffer = nullptr;
		HR(swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
		assert(backBuffer);

		//Use a generated UID so that filenames are unique
		UID imageFileID = GenerateUID();
		std::wstring imageFile = L"Screenshots/" + std::to_wstring(imageFileID) + L".jpg";
		HR(SaveWICTextureToFile(context, backBuffer, GUID_ContainerFormatJpeg, imageFile.c_str()));
		debugMenu.AddNotification(L"Screen shot taken.");
	}
}

void Renderer::MeshIconImageCapture()
{
	Actor* actor = WorldEditor::GetPickedActor();
	std::vector<MeshComponent*> meshComponents = actor->GetComponentsOfType<MeshComponent>();

	//@Todo: isn't working with multiple meshes nicely
	if (!meshComponents.empty())
	{
		MeshComponent* mesh = meshComponents.front();

		ID3D11Texture2D* backBuffer = nullptr;
		HR(swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
		assert(backBuffer);

		std::wstring imageFile = L"Icons/MeshIcons/" + VString::stows(mesh->meshComponentData.filename) + L".jpg";
		HR(SaveWICTextureToFile(context, backBuffer, GUID_ContainerFormatJpeg, imageFile.c_str()));
		debugMenu.AddNotification(L"Mesh Icon created.");
	}
}

void Renderer::PlayerPhotoCapture(std::wstring outputFilename)
{
	debugMenu.debugNotifications.clear();

	ID3D11Texture2D* backBuffer = nullptr;
	HR(swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
	assert(backBuffer);

	std::wstring imageFile = L"Textures/" + outputFilename;
	HR(SaveWICTextureToFile(context, backBuffer, GUID_ContainerFormatJpeg, imageFile.c_str()));
	Log("Photo taken [%S]", imageFile.c_str());
}

void SetRenderPipelineStates(MeshComponent* mesh)
{
	Material* material = mesh->material;
	PipelineStateObject& pso = mesh->pso;

	if (Renderer::drawAllAsWireframe)
	{
		context->RSSetState(rastStateWireframe);
	}
	else if (material->rastState)
	{
		context->RSSetState(material->rastState->data);
	}

	constexpr FLOAT blendState[4] = { 0.f };
	context->OMSetBlendState(material->blendState->data, blendState, 0xFFFFFFFF);

	context->VSSetShader(material->GetVertexShader(), nullptr, 0);
	context->PSSetShader(material->GetPixelShader(), nullptr, 0);

	context->PSSetSamplers(0, 1, &material->sampler->data);
	SetShaderResourceFromMaterial(0, material);

	SetVertexBuffer(pso.vertexBuffer);

	cbMaterial->Map(&material->materialShaderData);
	cbMaterial->SetPS();
}

void SetRenderPipelineStatesForShadows(MeshComponent* mesh)
{
	Material* material = mesh->material;
	PipelineStateObject& pso = mesh->pso;

	context->RSSetState(rastStateMap["shadow"]->data);

	ShaderItem* shader = ShaderItems::Shadow;

	context->VSSetShader(shader->GetVertexShader(), nullptr, 0);
	context->PSSetShader(shader->GetPixelShader(), nullptr, 0);

	context->IASetVertexBuffers(0, 1, &pso.vertexBuffer->data, &Renderer::stride, &Renderer::offset);
}

void SetShaders(ShaderItem* shaderItem)
{
	context->VSSetShader(shaderItem->GetVertexShader(), nullptr, 0);
	context->PSSetShader(shaderItem->GetPixelShader(), nullptr, 0);
}

void SetRastState(std::string rastStateName)
{
	if (Renderer::drawAllAsWireframe)
	{
		context->RSSetState(rastStateWireframe);
		return;
	}

	auto& rastState = rastStateMap[rastStateName];
	context->RSSetState(rastState->data);
}

void SetBlendState(std::string blendStateName)
{
	auto& blendState = blendStateMap[blendStateName];
	const float factor[4] = {};
	context->OMSetBlendState(blendState->data, factor, 0xFFFFFFFF);
}

void SetConstantBufferVertexPixel(uint32_t shaderRegister, ID3D11Buffer* constantBuffer)
{
	context->VSSetConstantBuffers(shaderRegister, 1, &constantBuffer);
	context->PSSetConstantBuffers(shaderRegister, 1, &constantBuffer);
}

void SetConstantBufferVertex(uint32_t shaderRegister, ID3D11Buffer* constantBuffer)
{
	context->VSSetConstantBuffers(shaderRegister, 1, &constantBuffer);
}

void SetConstantBufferPixel(uint32_t shaderRegister, ID3D11Buffer* constantBuffer)
{
	context->PSSetConstantBuffers(shaderRegister, 1, &constantBuffer);
}

void SetVertexBuffer(Buffer* vertexBuffer)
{
	context->IASetVertexBuffers(0, 1, &vertexBuffer->data, &Renderer::stride, &Renderer::offset);
}

void SetIndexBuffer(Buffer* indexBuffer)
{
	context->IASetIndexBuffer(indexBuffer->data, indexBufferFormat, 0);
}

void SetSampler(uint32_t shaderRegister, Sampler* sampler)
{
	context->PSSetSamplers(shaderRegister, 1, &sampler->data);
}

void SetShaderResourceFromMaterial(uint32_t shaderRegister, Material* material)
{
	//Testing code for normal map SRV set
	/*auto normalMapTexture = textureSystem.FindTexture2D("wall_normal_map.png");
	auto normalMapSRV = normalMapTexture->GetSRV();
	context->PSSetShaderResources(normalMapTexureRegister, 1, &normalMapSRV);*/

	auto textureSRV = material->texture->GetSRV();
	context->PSSetShaderResources(shaderRegister, 1, &textureSRV);
}

void SetShaderResourcePixel(uint32_t shaderRegister, std::string textureName)
{
	auto texture = TextureSystem::FindTexture2D(textureName);
	auto textureSRV = texture->GetSRV();
	context->PSSetShaderResources(shaderRegister, 1, &textureSRV);
}

void RenderPostProcess()
{
	auto postProcessInstance = PostProcessInstance::system.GetFirstActor();
	if (postProcessInstance == nullptr) return;
	if (!postProcessInstance->IsActive()) return;

	SetNullRTV();

	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;

	context->RSSetState(rastStateMap[RastStates::solid]->data);

	context->IASetInputLayout(nullptr);
	context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

	SetShaders(ShaderItems::PostProcess);

	//Set constant buffer data
	cbPostProcess->Map(&postProcessInstance->postProcessData);
	cbPostProcess->SetPS();

	context->PSSetShaderResources(0, 1, &postSRV);
	SetSampler(0, RenderUtils::GetDefaultSampler());

	UINT frameIndex = swapchain->GetCurrentBackBufferIndex();
	context->OMSetRenderTargets(1, &rtvs[frameIndex], dsv);

	const float clearColour[4] = { 0.f, 0.f, 0.f, 0.f };
	context->ClearRenderTargetView(rtvs[frameIndex], clearColour);

	//@Todo: there's an idea from the old post processing code that I liked.
	//Basically instead of drawing a stupid quad and rendering a texture onto it, copy to the backbuffer like below:
	//context->CopyResource(backBuffer, postBuffer);
	//There are typecasting errors with this (e.g. 16_FLOAT can't be cast to 8_UNORM) but it felt better.

	context->Draw(6, 0);

	context->PSSetShaderResources(0, 1, &nullSRV);
}

void CreatePostProcessRenderTarget()
{
	if (postBuffer) postBuffer->Release();
	if (postRTV) postRTV->Release();
	if (postSRV) postSRV->Release();

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.SampleDesc.Count = 1;
	desc.Width = viewport.Width;
	desc.Height = viewport.Height;

	HR(device->CreateTexture2D(&desc, nullptr, &postBuffer));
	assert(postBuffer);

	HR(device->CreateRenderTargetView(postBuffer, nullptr, &postRTV));
	HR(device->CreateShaderResourceView(postBuffer, nullptr, &postSRV));
}

RastState* Renderer::GetRastState(std::string rastStateName)
{
	return rastStateMap[rastStateName].get();
}

BlendState* Renderer::GetBlendState(std::string blendStateName)
{
	return blendStateMap[blendStateName].get();
}

void Renderer::AddDebugDrawOrientedBox(DirectX::BoundingOrientedBox& orientedBox)
{
	debugOrientedBoxesOnTimerToRender.emplace_back(orientedBox);
}

void Renderer::ClearBounds()
{
	debugOrientedBoxesOnTimerToRender.clear();
}

void Renderer::AddDebugLine(Line& line)
{
	Vertex v1{}, v2{};
	v1.pos = line.p1;
	v2.pos = line.p2;
	debugLines.emplace_back(v1);
	debugLines.emplace_back(v2);
}

struct Texel
{
	XMFLOAT4 colour = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.f);
	XMFLOAT3 pos = XMFLOAT3(0.f, 0.f, 0.f);
};
