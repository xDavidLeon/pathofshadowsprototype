#pragma once
#include "entity.h"
#include "component.h"
#include <string>
#include <map>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <iostream>
#include "component_transform.h"
#include <set>

static int maxNumOfEntities = 9999;

class EntityManager
{
public:
	static EntityManager &get()
	{
		static EntityManager ts;
		return ts;
	}

	int			generateNewEid(void);
	Entity*		createEntity(void);
	void		enableEntity(const std::string& e_name, bool enabled);
	void		addComponent(Component	*	component,	Entity	*	entity);
	//void		removeComponent(Component* component, Entity	*	entity);
	void		removeEntity(Entity	*	entity, bool destroy_childrens = true);
	Entity*		getFirstEntityPosessingComponentOfClass(const std::string& className);
	void		releaseAll(void);
	Entity*		getEntityWithId(int id);
	Entity*		getEntityWithName(const std::string& e_name);
	void		addEntityByName(Entity* entity);

	std::set<Component*>*	getAllComponentsOfEntity(Entity* entity);

	void deleteComponentOfEntity(Entity* entity, Component* component);

	//funciones "atajo"
	const btVector3& getPlayerPos();

	template<class componentClass>
		componentClass* getComponent(Entity* entity)
		{
			return (componentClass*)getComponentOfClass(typeid(componentClass).name(), entity);
		}

	template<class componentClass>
		std::map<Entity*,Component*>* getAllEntitiesPosessingComponent()
		{
			return getAllEntitiesPosessingComponentOfClass(typeid(componentClass).name());
		}

private:
	std::map<int, Entity*> _entities;
	std::map<std::string, Entity*> _entitiesByName; //La idea es que en el futuro esto substituya lo anterior

	// Devuelve map de entidades y sus comp. que contienen el componente indicado
	std::map<const std::string,std::map<Entity*,Component*>*> _componentsByClass;
	std::map<Entity*,std::set<Component*>*> _componentsOfEntities;
	int _lowestUnassignedEid;

	Component*						getComponentOfClass(const std::string& className,	Entity	*	entity);
	std::map<Entity*,Component*>*	getAllEntitiesPosessingComponentOfClass(const std::string& className);
	
	EntityManager(void);
	~EntityManager(void);
};

