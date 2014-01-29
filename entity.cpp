#include "entity.h"
#include "component.h"
#include <typeinfo>

Entity::Entity(int e)
{
	eid = e;
	name = "no name";
	type = "DEFAULT";
	enabled = true;
}

Entity::~Entity(void)
{
}

//Component* Entity::getComponent(const std::string className)
//{
//	if (_components.find(className) == _components.end()) return NULL;
//
//	return _components.at(className);
//}

//void Entity::addComponent(const std::string className, Component * component)
//{
//	//_components.insert(std::pair<const std::string,Component*>(className,component));
//}
//
//void Entity::removeComponent(const std::string className)
//{
//	//_components.erase(className);
//}