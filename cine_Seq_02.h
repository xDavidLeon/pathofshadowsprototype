#ifndef CINE_SEQ_02
#define CINE_SEQ_02

#include "behaviour_tree.h"
#include "entity.h"
#include <deque>
#include "component_transform.h"
#include "counter_clock.h"
#include "bt_goddess.h"

class CineSeq02 : public BehaviourTree
{
	CounterClock _clock;
	BTGoddess *_btGoddess;
	bool _event;

public:
	CineSeq02(Entity* entity);
	~CineSeq02();

	void create();
	void init(){};
	void render();

	//Actions
	int lightsOff();
	int goddessAppears();
	int shadowRecharges();
	int goddessGivesPowers();
	int audio3();
	int tutorialTeleport();
	int die();
};

#endif