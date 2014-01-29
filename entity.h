#ifndef INC_ENTITY_H_
#define INC_ENTITY_H_
#include <map>
#include <string>

class Component;
class Entity
{
public:
	Entity(int e);
	~Entity(void);

	int		eid;
	std::string name;
	std::string type;

	bool enabled;

	//void addComponent(const std::string className, Component * component);
	//void removeComponent(const std::string className);
	//Component* getComponent(const std::string className);
private:
	//std::map<const std::string,Component*> _components;
};

#endif