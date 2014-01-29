#ifndef CINE_SEQ_05_2
#define CINE_SEQ_05_2

#include "behaviour_tree.h"
#include "entity.h"
#include "counter_clock.h"

class CineSeq05_2 : public BehaviourTree
{
	CounterClock _clock;

public:
	CineSeq05_2(Entity* entity);
	~CineSeq05_2();

	void create();
	void init(){};
	void render();

	//Actions
	int waitToTalk();
	int die();
};

#endif