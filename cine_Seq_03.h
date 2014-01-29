#ifndef CINE_SEQ_03
#define CINE_SEQ_03

#include "bt_goddess.h"
#include "entity.h"
#include "component_transform.h"
#include "counter_clock.h"

class CineSeq03 : public BehaviourTree
{
	CounterClock _clock;
	BTGoddess *_btGoddess;

public:
	CineSeq03(Entity* entity);
	~CineSeq03();

	void create();
	void init(){};
	void render();

	//Actions
	int audio4();
	int goddessTeleport();
	int die();
};

#endif