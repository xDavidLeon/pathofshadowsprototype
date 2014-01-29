#ifndef INC_COMPONENT_H_
#define INC_COMPONENT_H_

enum COMP_TYPE {
	MODEL,
	TRANSFORM
};

class Entity;
class Component
{
public:
	virtual void init(){};
	virtual void update(float delta){};
	virtual void lateUpdate(float delta){};
	Component(Entity * e);
	virtual ~Component() {}

	bool enabled;
	Entity* entity;
};

#endif