#pragma once

#include "Sprite.h"

struct SpriteSheetEmitter;
struct ID3D11Buffer;
struct ID3D11DeviceContext;

//Sprite and text rendering for D3D11
//RRef:http://www.d3dcoder.net/Data/Resources/SpritesAndText.pdf

//SpriteSystem is used for rendering both widget and particle images (screen space vs world space).
//Ideally you would seperate on-screen image and particle rendering out, but the quad building and buffers
//are generic enough to be shared between the two.
namespace SpriteSystem
{
	void Init();
	void Reset();
	void CreateScreenSprite(Sprite sprite);
	void BuildSpriteQuadForViewportRendering(const Sprite& sprite);
	void BuildSpriteQuadForSpriteSheetRendering(const Sprite& sprite);
	void BuildSpriteQuadForParticleRendering();
	void UpdateAndSetSpriteBuffers(ID3D11DeviceContext* context);
	std::vector<Sprite>& GetScreenSprites();
};
