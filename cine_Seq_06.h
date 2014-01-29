#ifndef CINE_SEQ_06
#define CINE_SEQ_06

#include "behaviour_tree.h"
#include "entity.h"
#include "counter_clock.h"

class CineSeq06 : public BehaviourTree
{
	CounterClock _clock;

public:
	CineSeq06(Entity* entity);
	~CineSeq06();

	void create();
	void init(){};
	void render();

	//Actions
	int camera5();
	int die();
};

#endif