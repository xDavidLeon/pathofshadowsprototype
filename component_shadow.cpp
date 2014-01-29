#include "component_shadow.h"
#include "entity_factory.h"
#include "entity_manager.h"

ShadowComponent::ShadowComponent(Entity* e, D3DXVECTOR3 upVector, int sid) : Component(e)
{
	radius = 0.0f;
	normal = upVector;
	state = msStates::GROWING;
	shadow_id = sid;
	TransformComponent * t = EntityManager::get().getComponent<TransformComponent>(e);
	//Entity * p = EntityFactory::get().createParticleEffect(D3DXVECTOR3(t->getPosition()),ParticleEffectComponent::PARTICLE_EFFECT::SHADOW_CREATION);
	//particle = EntityManager::get().getComponent<ParticleEffectComponent>(p);
	////particle->aabb_radius = radius;
	//particle->enabled = false;
	//particle->stop();
}


ShadowComponent::~ShadowComponent(void)
{
}

void ShadowComponent::grow()
{
	if(state == msStates::SHRINKING) state = msStates::GROWING;
}

void ShadowComponent::stopGrowing()
{
	if(state == msStates::GROWING) state = msStates::SHRINKING;
}

void ShadowComponent::die()
{
	state = msStates::DYING;
}

bool ShadowComponent::canDie()
{
	return state == msStates::SHRINKING;
}
