#include "system_animation.h"
#include "entity.h"
#include <map>
#include "component.h"
#include "entity_manager.h"
#include "component_animation.h"
#include "component_model.h"

AnimationSystem::AnimationSystem(void)
{
}


AnimationSystem::~AnimationSystem(void)
{
}

void AnimationSystem::update(float delta)
{
	//Se obtienen todas las entidades con componente animation
	std::map<Entity*,Component*>* entitiesWithAnimation = EntityManager::get().getAllEntitiesPosessingComponent<AnimationComponent>();
	//Se llama el update de los componentes animation
	std::map<Entity*,Component*>::iterator iter;
	//#pragma omp parallel for
	for (iter = entitiesWithAnimation->begin(); iter != entitiesWithAnimation->end(); ++iter)
	{
		Entity * e = iter->first;
		AnimationComponent* animation = (AnimationComponent*)iter->second;
		if(!e->enabled) continue;
		if( animation->enabled)
		{
			if(e->type == "ANIMATED" && !isInFrustum(e) ) continue;

			//dbg("Animated entity updating %s\n", e->name.c_str());
			animation->update(delta);
		}
	}
}

bool AnimationSystem::isInFrustum( Entity* entity )
{
	TCamera camera = CameraSystem::get().getCurrentCamera();

	ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(entity);
	TransformComponent* transform = EntityManager::get().getComponent<TransformComponent>(entity);
	D3DXMATRIX d3dxMatrix;
	convertBulletTransform( transform->transform, d3dxMatrix);
	TAABB aabb = model->getMesh()->aabb;
	aabb = aabb.getRotatedBy(d3dxMatrix);

	if (camera.frustum.isInside(aabb) == TFrustum::OUT_OF_FRUSTUM)
		return false;

	btTransform * world = transform->transform;
	// Size + dist culling
	float r = aabb.getRadius();
	D3DXVECTOR3 camPos = camera.getPosition();
	float dist = world->getOrigin().distance(camera.getPosition());
	float deg = atan(r/dist);
			
	if (deg < (camera.getFov()/50.0f))
		return false;
			

	return true;
}

void AnimationSystem::render()
{
	////Se obtienen todas las entidades con componente animation
	//std::map<Entity*,Component*>* entitiesWithAnimation = EntityManager::get().getAllEntitiesPosessingComponent<AnimationComponent>();

	////Se llama el update de los componentes animation
	//std::map<Entity*,Component*>::iterator iter;
	//for (iter = entitiesWithAnimation->begin(); iter != entitiesWithAnimation->end(); ++iter)
	//{
	//	((AnimationComponent*)iter->second)->render();
	//}
}

void AnimationSystem::renderDebug()
{
	//Se obtienen todas las entidades con componente animation
	std::map<Entity*,Component*>* entitiesWithAnimation = EntityManager::get().getAllEntitiesPosessingComponent<AnimationComponent>();

	//Se llama el update de los componentes animation
	std::map<Entity*,Component*>::iterator iter; 

	//**************************************** xu, saltamos shadow
	iter = entitiesWithAnimation->begin();
	//++iter;
	//++iter;
	//********************************************

	//((AnimationComponent*)iter->second)->renderDebug();

	size_t index = 0;

	for (iter; iter != entitiesWithAnimation->end(); ++iter)
	{
		if(iter->first->type == "PLAYER")
		{
			((AnimationComponent*)iter->second)->renderDebug(index);
			index++;
		}
	}

	/*for (iter = entitiesWithAnimation->begin(); iter != entitiesWithAnimation->end(); ++iter)
	{
		((AnimationComponent*)iter->second)->renderDebug();
	}*/
}
