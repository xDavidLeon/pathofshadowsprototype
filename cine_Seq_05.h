#ifndef CINE_SEQ_05
#define CINE_SEQ_05

#include "behaviour_tree.h"
#include "entity.h"
#include "counter_clock.h"

class CineSeq05 : public BehaviourTree
{
	CounterClock _clock;

public:
	CineSeq05(Entity* entity);
	~CineSeq05();

	void create();
	void init(){};
	void render();

	//Actions
	int openDoor();
	int fadeOut();
	int loadNextLevel();
	int die();
};

#endif