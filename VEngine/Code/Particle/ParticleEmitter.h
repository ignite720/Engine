#pragma once

#include "Components/SpatialComponent.h"
#include "Components/ComponentSystem.h"
#include "Particle.h"
#include "Render/ShaderItem.h"

class Material;

struct ParticleEmitter : SpatialComponent
{
	COMPONENT_SYSTEM(ParticleEmitter)

	std::vector<Particle> particles;

	Material* material = nullptr;

	XMFLOAT3 direction = XMFLOAT3(0.f, 0.f, 0.f);

	//The Float2's here are acting as a range [x = min, y = max]
	XMFLOAT2 moveSpeed = XMFLOAT2(1.f, 1.f);
	XMFLOAT2 lifetime = XMFLOAT2(1.f, 1.f);
	XMFLOAT2 spawnRate = XMFLOAT2(0.3f, 0.3f);
	XMFLOAT2 spawnRadius = XMFLOAT2(0.f, 1.f);
	XMFLOAT2 rotation = XMFLOAT2(0.f, 0.f);
	XMFLOAT2 rotateSpeed = XMFLOAT2(0.f, 0.f);

	//Seperate from individual particle lifetime, Emitter destroys itself when it reaches a max.
	float emitterLifetime = 0.f;

private:
	float spawnTimer = 0.f;
	float emitterLifetimeTimer = 0.f;

public:
	ParticleEmitter(std::string textureFilename = "test.png", ShaderItem* shaderItem = ShaderItems::DefaultClip);
	void CreateParticle(Particle particle);
	virtual void Tick(float deltaTime) override;
	virtual void Create() override;
	virtual Properties GetProps() override;
};
