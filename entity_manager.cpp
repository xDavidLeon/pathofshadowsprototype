#include "entity_manager.h"
#include <assert.h>	
#include <algorithm>
#include "world.h"
#include <string>

#include "component_animation.h"
#include "component_automat.h"
#include "component_billboard.h"
#include "component_charcontroller.h"
#include "component_light.h"
#include "component_model.h"
#include "component_particle_effect.h"
#include "component_rigidbody.h"
#include "component_trigger.h"

EntityManager::EntityManager(void)
{
	_lowestUnassignedEid = 0;
}


EntityManager::~EntityManager(void)
{
	releaseAll();
}

void EntityManager::releaseAll(void)
{
	std::map<int, Entity*>::iterator iter;

	while(_entities.size())
	{
		iter = _entities.begin();
		removeEntity(iter->second, false);
	}

	std::map<const std::string,std::map<Entity*,Component*>*>::iterator iter1; 

	for(iter1=_componentsByClass.begin(); iter1!=_componentsByClass.end(); iter1++)
	{
		iter1->second->clear();
	}

	_componentsByClass.clear();

	//std::map<Entity*,std::set<Component*>*>::iterator iter2;

	_componentsOfEntities.clear();
}

const btVector3& EntityManager::getPlayerPos()
{
	return getComponent<TransformComponent>(World::instance()->getPlayer())->getPosition();
}

int EntityManager::generateNewEid(void)
{
	if (_lowestUnassignedEid < maxNumOfEntities) {
        return _lowestUnassignedEid++;
    } else {
        for (int i = 0; i < maxNumOfEntities; ++i) {
            if (_entities[i] != NULL) {
                return i;
            }
        }
        assert ("No available Entity IDs!");
        return 0;
    }
}

Entity* EntityManager::createEntity(void)
{
	int eid = generateNewEid();
	Entity* e = new Entity(eid);
	_entities.insert(std::pair<int, Entity*>(e->eid, e));
	return e;
}

Entity* EntityManager::getEntityWithName(const std::string & e_name)
{
	if(_entitiesByName.find(e_name) != _entitiesByName.end())
		return _entitiesByName.at(e_name);

	return NULL;
}

void EntityManager::enableEntity(const std::string& e_name, bool enabled)
{
	if(_entitiesByName.find(e_name) != _entitiesByName.end())
		_entitiesByName.at(e_name)->enabled = enabled;
}

void EntityManager::addComponent(Component	*	component,	Entity	*	entity)
{
	std::map<Entity*,Component*> * components;
	const std::string className = typeid(*component).name();

	if (_componentsByClass.find(className) != _componentsByClass.end())
		components = _componentsByClass[className];
	else
	{
		components = new std::map<Entity*,Component*>();
		_componentsByClass[className] = components;
	}
	components->insert(std::pair<Entity*,Component*>(entity,component));


	if (_componentsOfEntities.find(entity) == _componentsOfEntities.end())
		_componentsOfEntities.insert(std::pair<Entity*,std::set<Component*>*>(entity, new std::set<Component*>()));

	if(_componentsOfEntities.at(entity)->find(component) == _componentsOfEntities.at(entity)->end())
		_componentsOfEntities.at(entity)->insert(component);
}

Component	*	EntityManager::getComponentOfClass(const std::string& className,	Entity	*	entity)
{
	if (_componentsByClass.find(className) == _componentsByClass.end()) return NULL;
	std::map<Entity*,Component*>* myMap = _componentsByClass[className];
	if (myMap->find(entity) == myMap->end()) return NULL;
	return myMap->at(entity);
}

void EntityManager::removeEntity(Entity	*	entity, bool destroy_childrens)
{
	if (entity == NULL) return;
	TransformComponent * t = EntityManager::get().getComponent<TransformComponent>(entity);
	if (t != NULL && destroy_childrens) t->destroyChildren();

	//LightComponent * l = EntityManager::get().getComponent<LightComponent>(entity);
	//if (l != NULL) LightSystem::get().removeLight(entity, l->getType());

	std::map<const std::string,std::map<Entity*,Component*>*>::iterator iter;
	for (iter = _componentsByClass.begin(); iter != _componentsByClass.end(); ++iter)
	{
		std::map<Entity*,Component*> * components = iter->second;
		if (components->find(entity) != components->end())
		{
			delete components->at(entity);
			components->erase(entity);
		}
	}

	if(_componentsOfEntities.find(entity) != _componentsOfEntities.end())
	{
		delete _componentsOfEntities.at(entity);
		_componentsOfEntities.erase(entity);
	}

	if (entity == NULL) return;
	if(_entities.find(entity->eid) != _entities.end())
	{
		_entities.erase(entity->eid);
	}
	
	if(_entitiesByName.find(entity->name) != _entitiesByName.end())
	{
		_entitiesByName.erase(entity->name);
	}

	delete entity;
}

//void EntityManager::removeComponent(Component* component, Entity	*	entity)
//{
	//std::map<Entity*,Component*> * components;
	//const std::string className = typeid(*component).name();

	//if (_componentsByClass.find(className) != _componentsByClass.end())
	//	components = _componentsByClass[className];
	//else
	//{
	//	components = new std::map<Entity*,Component*>();
	//	_componentsByClass[className] = components;
	//}
	//components->erase(std::remove(components->begin(), components->end(), component));
	//entity->removeComponent(className);
//}

Entity*		EntityManager::getFirstEntityPosessingComponentOfClass(const std::string& className)
{
	if(_componentsByClass.find(className) == _componentsByClass.end()) return NULL;
	else return _componentsByClass[className]->begin()->first;
}

std::map<Entity*,Component*>* EntityManager::getAllEntitiesPosessingComponentOfClass(const std::string& className)
{
	if(_componentsByClass.find(className) == _componentsByClass.end()) return NULL;
	else return _componentsByClass[className];

 /*   if (components) {
		std::vector<Entity*> * retval = new std::vector<Entity*>(components->size());
		std::map<Entity*,Component*>::iterator iter;
		for (iter = components->begin(); iter != components->end(); ++iter)
			retval->push_back(iter->first);
        return retval;
    } else {
        return new std::vector<Entity*>();
    }*/
}

std::set<Component*>*	EntityManager::getAllComponentsOfEntity(Entity* entity)
{
	if(_componentsOfEntities.find(entity) != _componentsOfEntities.end())
		return _componentsOfEntities.at(entity);
	else return NULL;
}

//WARNING: ambos punteros deben ser veridicos
void EntityManager::deleteComponentOfEntity(Entity* entity, Component* component)
{
	//borrar la constancia en _componentsByClass
	//...no encuentro ninguna manera guay de saber de que tipo es un componente dado su puntero a Component (superclase), asi que vamos comparando..
	if(dynamic_cast<AnimationComponent*>(component))				_componentsByClass.at(typeid(AnimationComponent).name())->erase(entity);
	else if(dynamic_cast<AutomatComponent*>(component))				_componentsByClass.at(typeid(AutomatComponent).name())->erase(entity);
	else if(dynamic_cast<BillboardComponent*>(component))			_componentsByClass.at(typeid(BillboardComponent).name())->erase(entity);
	else if(dynamic_cast<CameraComponent*>(component))				_componentsByClass.at(typeid(CameraComponent).name())->erase(entity);
	else if(dynamic_cast<CharacterControllerComponent*>(component)) _componentsByClass.at(typeid(CharacterControllerComponent).name())->erase(entity);
	else if(dynamic_cast<LightComponent*>(component))				_componentsByClass.at(typeid(LightComponent).name())->erase(entity);
	else if(dynamic_cast<ModelComponent*>(component))				_componentsByClass.at(typeid(ModelComponent).name())->erase(entity);
	else if(dynamic_cast<ParticleEffectComponent*>(component))		_componentsByClass.at(typeid(ParticleEffectComponent).name())->erase(entity);
	else if(dynamic_cast<PlayerControllerComponent*>(component))	_componentsByClass.at(typeid(PlayerControllerComponent).name())->erase(entity);
	else if(dynamic_cast<RigidbodyComponent*>(component))			_componentsByClass.at(typeid(RigidbodyComponent).name())->erase(entity);
	else if(dynamic_cast<ShadowActionsComponent*>(component))		_componentsByClass.at(typeid(ShadowActionsComponent).name())->erase(entity);
	else if(dynamic_cast<TransformComponent*>(component))			_componentsByClass.at(typeid(TransformComponent).name())->erase(entity);
	else if(dynamic_cast<TriggerComponent*>(component))				_componentsByClass.at(typeid(TriggerComponent).name())->erase(entity);

	//borrar la constancia en _componentsOfEntities
	_componentsOfEntities.at(entity)->erase(component);
	
	//eliminar componente (en este punto no deberia estar referenciado en ninguna estructura de datos)
	delete component;
}


//void EntityManager::update(float delta)
//{
//	std::map<const std::string,std::map<Entity*,Component*>*>::iterator i_component;
//	std::map<Entity*,Component*>::iterator i_entity_with_component;
//	std::map<Entity*,Component*>* entitiesWithComponent = NULL;
//
//	//Para cada tipo de componente...
//	for (i_component = _componentsByClass.begin(); i_component != _componentsByClass.end(); ++i_component)
//	{
//		entitiesWithComponent = i_component->second;
//
//		//Para cada entidad con ese componente...
//		for (i_entity_with_component = entitiesWithComponent->begin(); i_entity_with_component != entitiesWithComponent->end(); ++i_entity_with_component)
//		{
//			i_entity_with_component->second->update(delta);
//		}
//	}
//
//}

Entity* EntityManager::getEntityWithId(int id)
{
	if(id >= _entities.size()) return NULL; //La id puede ser como maximo _entities.size()-1
	
	return _entities.at(id);
}

void EntityManager::addEntityByName(Entity* entity)
{
	if(_entitiesByName.find(entity->name) == _entitiesByName.end())
		_entitiesByName.insert(std::pair<std::string, Entity*>(entity->name, entity));
	else 
		int kk=23;
}
