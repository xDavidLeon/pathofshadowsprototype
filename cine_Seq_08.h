#ifndef CINE_SEQ_08
#define CINE_SEQ_08

#include "behaviour_tree.h"
#include "entity.h"
#include "counter_clock.h"

class CineSeq08 : public BehaviourTree
{
	CounterClock _clock;

public:
	CineSeq08(Entity* entity);
	~CineSeq08();

	void create();
	void init(){};
	void render();

	//Actions
	int openDoor();
	int fadeOut();
	int die();
};

#endif