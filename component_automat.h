#ifndef AUTOMAT_COMPONENT_H
#define AUTOMAT_COMPONENT_H

#include "automat.h"
#include "component.h"

enum automatTypes
{
	PLAYER
} ;

class AutomatComponent : public Component
{
	Automat* _automat;
	automatTypes _automatType;

public:
	AutomatComponent(Entity* entity, automatTypes automatType);
	~AutomatComponent(void);

	void init();

	Automat* getAutomat() const{ return _automat; }
	void update(float delta);
	void render(); //for debug
	
};

#endif