#pragma once

#include "Components/SpatialComponent.h"
#include "Components/ComponentSystem.h"
#include "Particle.h"
#include "Render/ShaderItem.h"
#include "ParticleData.h"

class Material;

class ParticleEmitter : public SpatialComponent
{
public:
	COMPONENT_SYSTEM(ParticleEmitter);

	ParticleEmitter(std::string textureFilename = "test.png", std::string shaderItemName = "DefaultClip");
	void Create() override;
	void Destroy() override;
	void Start() override;
	void Tick(float deltaTime) override;
	Properties GetProps() override;

	Material& GetMaterial() { return *_material; }
	void SetAlpha(float alpha);
	float GetAlpha();

	void SetTexture(std::string_view textureFilename);

	auto GetParticleCount() const { return particles.size(); }
	std::vector<Particle> particles;

	ParticleData particleData;

	//Separate from individual particle lifetime, Emitter destroys itself when it reaches a max.
	float emitterLifetime = 0.f;

private:
	Material* _material = nullptr;

	float spawnTimer = 0.f;
	float emitterLifetimeTimer = 0.f;
};
