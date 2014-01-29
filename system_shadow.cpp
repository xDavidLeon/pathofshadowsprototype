#include "system_shadow.h"
#include "d3ddefs.h"
#include "world.h"
#include "entity_factory.h"
#include "component_shadow.h"
#include "entity_manager.h"

ShadowSystem::ShadowSystem(void)
{
	_created_shadows = 0;
	_activeShadows = 0;
	_maxActiveShadows = 2;
}

ShadowSystem::~ShadowSystem(void)
{
}

Entity* ShadowSystem::createShadow(const D3DXVECTOR3 & pos, const D3DXVECTOR3 & normal)
{
	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ShadowComponent>();

	Entity * e;

	// Tengo espacio para una sombra? Entonces crearla
	if (!entities)
	{
		e = EntityFactory::get().createMagicShadow(pos,normal,_created_shadows);
		_created_shadows++;
		_activeShadows++;
		return e;
	}

	int num_entities = entities->size();
	if (num_entities < _maxActiveShadows)
	{
		e = EntityFactory::get().createMagicShadow(pos,normal,_created_shadows);
		_created_shadows++;
		_activeShadows++;
		return e;
	}

	// sino, buscar un hueco, eliminar la mas antigua
	int oldest_id = _created_shadows;
	ShadowComponent * oldest_entity = NULL;
	
	std::map<Entity*,Component*>::iterator iter;
	for (iter = entities->begin(); iter != entities->end(); ++iter)
	{
		Entity* shadowE = iter->first;
		ShadowComponent * shadowC = (ShadowComponent*) iter->second;
		if (shadowC->shadow_id <= oldest_id && shadowC->state != msStates::DYING)
		{
			oldest_id = shadowC->shadow_id;
			oldest_entity = shadowC;
		}
	}
	if (oldest_entity != NULL) oldest_entity->die();
	//EntityManager::get().removeEntity(oldest_entity);
	e = EntityFactory::get().createMagicShadow(pos,normal,_created_shadows);
	_activeShadows++;
	_created_shadows++;

	return e;	
}

void ShadowSystem::update(float delta)
{
	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ShadowComponent>();
	if(!entities) return;

	std::map<Entity*,Component*>::iterator iter;
	for (iter = entities->begin(); iter != entities->end(); ++iter)
	{
		Entity* shadowE = iter->first;
		ShadowComponent * shadowC = (ShadowComponent*) iter->second;
		
		//Modificamos radio segun estado
		switch(shadowC->state)
		{
			case msStates::GROWING:
				if (shadowC->radius < visible_radius){ shadowC->radius += grow_radius*3.0f; }
				else if(shadowC->radius >= max_radius){ shadowC->state = msStates::SHRINKING; break; }
				else
				{
					//Aqui se comprueba que el radio de la sombra no entre en una pointlight. No va muy bien, pero mira
					const btVector3 &s_pos = EntityManager::get().getComponent<TransformComponent>(shadowE)->getPosition();
					if(!LightSystem::get().posInPointLight(s_pos, shadowC->radius*shadowC->radius))
						shadowC->radius += grow_radius;
				}
				break;
			case msStates::SHRINKING:	
				shadowC->radius -= shrink_radius; 
				if(shadowC->radius < visible_radius) 
					shadowC->state = msStates::DYING; //Si la sombra ya no nos oculta hacemos que encoja mas rapido
				break;
			case msStates::DYING:
				shadowC->radius = shrink_factor_fast*shadowC->radius;
				if(shadowC->radius < 0.01)
				{
					//shadowC->particle->destroy_on_finish = true;
					EntityManager::get().removeEntity(shadowE);
					_activeShadows--;
					return;
				}
				break;

		}
		//if (shadowC->radius > 0.25f) 
		//{
		//	if (shadowC->particle->enabled == false)
		//	{
		//		shadowC->particle->setRandomCenter(D3DXVECTOR3(-0.5f,0.0f,-0.5f),D3DXVECTOR3(0.5f,0.0f,0.5f));
		//		shadowC->particle->enabled = true;
		//		//shadowC->particle->setRandomStartLife();
		//		shadowC->particle->play();
		//	}
		//}
		//else shadowC->particle->stop();
		//shadowC->particle->aabb_radius = shadowC->radius;
	}
}

bool ShadowSystem::checkPosInShadows(const D3DXVECTOR3 & ipos, float offset) const
{
	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ShadowComponent>();
	if(!entities) return false;

	std::map<Entity*,Component*>::iterator iter;
	for (iter = entities->begin(); iter != entities->end(); ++iter)
	{
		Entity* shadowE = iter->first;
		ShadowComponent * shadowC = (ShadowComponent*) iter->second;
		TransformComponent * shadowT = EntityManager::get().getComponent<TransformComponent>(shadowE);

		if (shadowC->radius < visible_radius) continue;
		D3DXVECTOR3 dist = ipos - D3DXVECTOR3(shadowT->getPosition());
		if (D3DXVec3Length(&dist) < shadowC->radius-offset) return true;
	}

	return false;
}

bool ShadowSystem::checkPlayerInShadows() const
{
	return checkPosInShadows(D3DXVECTOR3(EntityManager::get().getPlayerPos() + btVector3(0,-0.6f,0)));
}

bool ShadowSystem::checkValidPosToTeleport(const D3DXVECTOR3 & ipos) const
{
	//Test posicion en alguna sombra magica
	bool inMagicShadow = checkPosInShadows(ipos);

	//test posicion en sombra estatica
	btVector3 pos_bt;
	convertDXVector3(ipos, pos_bt);
	return inMagicShadow || !LightSystem::get().posInDirectionalLight(pos_bt);
}

//Elimina todas las entidades con ShadowComponent
void ShadowSystem::destroyShadows()
{
	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ShadowComponent>();
	if(entities)
	{
		std::map<Entity*,Component*>::iterator iter;
		while (entities->size())
		{
			iter = entities->begin();
			EntityManager::get().removeEntity(iter->first);
		}
	}
}

