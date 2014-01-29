#ifndef BT_COMPONENT_H
#define BT_COMPONENT_H

#include "behaviour_tree.h"
#include "component.h"

enum btTypes
{
	GATEKEEPER,
	PATROLER,
	CROW,
	CINE_SEQ,
	GODDESS
};

class BTComponent : public Component
{
	Entity* _entity;
	BehaviourTree* _bt;
	btTypes _btType;

public:
	BTComponent(Entity* e, btTypes btType);
	~BTComponent(void);

	void init();

	void update(float delta);
	void render(); //for debug

	BehaviourTree* getBT() const{ return _bt; }
	void changeBT(BehaviourTree* newBT);
};

#endif