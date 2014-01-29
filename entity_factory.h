#pragma once
#include <btBulletCollisionCommon.h>
#include <d3dx9.h>
#include "component_camera.h"
#include "component_light.h"
#include "component_particle_effect.h"

class Entity;
class EntityFactory
{
public:

	static EntityFactory &get()
	{
		static EntityFactory ts;
		return ts;
	}

	Entity* createBlob(const D3DXVECTOR3 & position, TransformComponent * parent, float radius);
	Entity* createMagicShadow(const D3DXVECTOR3 & position, const D3DXVECTOR3 & upVector, int shadow_id = 0);
	Entity* createEnemy(const btTransform& trans);
	Entity* createCrow();
	Entity* createDirectionalLight(D3DXVECTOR3 & direction, D3DXCOLOR color, float intensity);
	Entity* createPointLight(btVector3 & position, D3DXCOLOR color, float intensity, float radius, bool isTorch = true);
	Entity* createSpotLight(D3DXVECTOR3 & position, D3DXVECTOR3 & direction, D3DXCOLOR color, float intensity, float lnear, float lfar, float lfov, bool generateShadows);
	Entity* createParticleEffect(const D3DXVECTOR3 & position, ParticleEffectComponent::PARTICLE_EFFECT effect_type, ModelComponent * model = NULL, D3DXVECTOR3 dest = D3DXVECTOR3(0,0,0),D3DXVECTOR3 vel = D3DXVECTOR3(0,0,0));
	Entity* createCamera(CAM_TYPE type, const btVector3& pos, const btVector3& target, float fov_in_degrees, float aspect_ratio,  float cam_near, float cam_far);
	void createCineSeq(int cine_id);
	void createGoddess();

private:
	EntityFactory(void);
	~EntityFactory(void);
};

