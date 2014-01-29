#include "system_light.h"
#include "component_transform.h"
#include "angular.h"
#include "component_light.h"
#include "system_physics.h"
#include "entity_manager.h"
#include "component_particle_effect.h"

LightSystem::LightSystem(void)
{
	_lightRaycastOffset = 0.5f;
}


LightSystem::~LightSystem(void)
{
}

void LightSystem::update(float delta)
{
	std::map<Entity*,Component*>* lights = EntityManager::get().getAllEntitiesPosessingComponent<LightComponent>();
	if (lights != NULL)
	{
		std::map<Entity*,Component*>::iterator iter;
		for (iter = lights->begin(); iter != lights->end(); ++iter)
		{
			Entity* light = iter->first;
			LightComponent* l = EntityManager::get().getComponent<LightComponent>(light);
			if (l->enabled) l->update(delta);
		}
	}
}

void LightSystem::addLight(Entity* light_e, LIGHT_TYPE type)
{
	switch(type)
	{
		case LIGHT_TYPE::LIGHT_POINT:
			_pointLights.push_back(light_e);
			break;

		case LIGHT_TYPE::LIGHT_DIRECTIONAL:
			_dirLights.push_back(light_e);
			break;

		case LIGHT_TYPE::LIGHT_SPOT:
			_spotLights.push_back(light_e);
			break;
	}
}

void LightSystem::removeLight(Entity* light_e, LIGHT_TYPE type)
{
	switch(type)
	{
		case LIGHT_TYPE::LIGHT_POINT:
			if (_pointLights.size() > 0)
			_pointLights.remove(light_e);
			break;

		case LIGHT_TYPE::LIGHT_DIRECTIONAL:
			if (_dirLights.size() > 0) _dirLights.remove(light_e);
			break;

		case LIGHT_TYPE::LIGHT_SPOT:
			if (_spotLights.size() > 0) 
			_spotLights.remove(light_e);
			break;
	}
}

bool LightSystem::posInPointLight(const btVector3& pos, float offsetSq)
{
	std::list<Entity*>::iterator light;

	for(light=_pointLights.begin(); light!=_pointLights.end(); light++)
	{
		const btVector3& lightPos = EntityManager::get().getComponent<TransformComponent>(*light)->getPosition();
		LightComponent* lightC = EntityManager::get().getComponent<LightComponent>(*light);
		if (lightC->isTorch == false) continue;
		if (lightC->light_intensity <= 0.25f) continue;
		if(manhattanDist(pos, lightPos) < maxRadiusMh) //test rapido: dist manhattan
		{
			if(pos.distance2(lightPos) < lightC->getRadiusSq() + offsetSq)
			{
				////ultima comprobacion: lanzar rayo hasta fuente de luz (hasta un poco antes por si est� dentro de una mesh)
				//btVector3 offset_v = pos-lightPos;  offset_v.normalize();  offset_v*_lightRaycastOffset;
				//if(!PhysicsSystem::get().checkCollision(pos, lightPos+offset_v, PhysicsSystem::get().colMaskVision))
					return true;
			}
		}
	}

	return false;
}

bool LightSystem::posInDirectionalLight(const btVector3& pos)
{
	std::list<Entity*>::iterator light;
	int maxRayDist = 100;

	for(light=_dirLights.begin(); light!=_dirLights.end(); light++)
	{
		btVector3 lightDir; convertDXVector3(EntityManager::get().getComponent<LightComponent>(*light)->light_direction, lightDir);
		if(!PhysicsSystem::get().checkCollision(pos, pos-lightDir*maxRayDist, PhysicsSystem::get().colMaskIllumination))
			return true;
	}

	return false;
}

//para debug
void LightSystem::renderDirLights(const btVector3& from_pos)
{
	std::list<Entity*>::iterator light;
	int maxRayDist = 100;

	for(light=_dirLights.begin(); light!=_dirLights.end(); light++)
	{
		btVector3 lightDir; convertDXVector3(EntityManager::get().getComponent<LightComponent>(*light)->light_direction, lightDir);
		const btVector3& to = from_pos-lightDir*100;
		//comprobamos colision. Si choca pintamos linea hasta colision
		btCollisionWorld::ClosestRayResultCallback rayCallback(from_pos,to);
		rayCallback.m_collisionFilterMask = PhysicsSystem::get().colMaskVision;
		PhysicsSystem::get().getCollisionWorld()->rayTest(from_pos, to, rayCallback);
		if(rayCallback.hasHit())
			drawLine_bt(from_pos, rayCallback.m_hitPointWorld, D3DCOLOR_ARGB(255, 255,255,255));	
		else
			drawLine_bt(from_pos, to, D3DCOLOR_ARGB(255, 255,255,255));	
	}
}

#include "system_sound.h"
void LightSystem::putOutTorch(const std::string& pointlight, const std::string& fireP, const std::string& smokeP, const std::string& billboard, float poTime)
{
	//pointlight
	Entity * e = EntityManager::get().getEntityWithName(pointlight);
	EntityManager::get().getComponent<LightComponent>(e)->setTargetRadius(0.0f, 1.0f);
	EntityManager::get().getComponent<LightComponent>(e)->setTargetIntensity(0.0f, 1.0f);
	//particulas
	Entity * e2 = EntityManager::get().getEntityWithName(fireP);
	EntityManager::get().getComponent<ParticleEffectComponent>(e2)->stop();
	EntityManager::get().getComponent<ParticleEffectComponent>(EntityManager::get().getEntityWithName(smokeP))->stop();
	//billboard
	EntityManager::get().removeEntity(EntityManager::get().getEntityWithName(billboard));
	std::string id = intToString(e2->eid);
	SoundSystem::SoundInfo * sound = SoundSystem::get().getSoundInfo(id);
	if (sound != NULL) sound->desired_volume = 0.0f;
}

void LightSystem::releaseLights()
{
	_dirLights.clear();
	_pointLights.clear();
	_spotLights.clear();
}

