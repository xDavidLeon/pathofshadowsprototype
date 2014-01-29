#ifndef CINE_SEQ_10
#define CINE_SEQ_10

#include "behaviour_tree.h"
#include "entity.h"
#include "counter_clock.h"

class CineSeq10 : public BehaviourTree
{
	CounterClock _clock;
	bool _event;

public:
	CineSeq10(Entity* entity);
	~CineSeq10();

	void create();
	void init(){};
	void render();

	//Actions
	int namesOnStones();
	int finalCredits();
	int die();
};

#endif