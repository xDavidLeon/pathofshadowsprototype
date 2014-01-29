#ifndef CINE_SEQ_07
#define CINE_SEQ_07

#include "behaviour_tree.h"
#include "entity.h"
#include "counter_clock.h"

class CineSeq07 : public BehaviourTree
{
	CounterClock _clock;

public:
	CineSeq07(Entity* entity);
	~CineSeq07();

	void create();
	void init(){};
	void render();

	//Actions
	int camera6();
	int die();
};

#endif