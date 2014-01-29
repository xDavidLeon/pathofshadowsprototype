#ifndef CINE_SEQ_09
#define CINE_SEQ_09

#include "behaviour_tree.h"
#include "entity.h"
#include "counter_clock.h"

class CineSeq09 : public BehaviourTree
{
	CounterClock _clock;
	bool _event;

public:
	CineSeq09(Entity* entity);
	~CineSeq09();

	void create();
	void init(){};
	void render();

	//Actions
	int openDoor();
	int fadeOut();
	int die();
};

#endif