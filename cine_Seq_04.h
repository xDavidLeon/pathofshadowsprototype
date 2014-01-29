#ifndef CINE_SEQ_04
#define CINE_SEQ_04

#include "bt_goddess.h"
#include "entity.h"
#include "counter_clock.h"

class CineSeq04 : public BehaviourTree
{
	CounterClock _clock;
	BTGoddess *_btGoddess;

public:
	CineSeq04(Entity* entity);
	~CineSeq04();

	void create();
	void init(){};
	void render();

	//Actions
	int audio6();
	int tutorialHideCorpse();
	int die();
};

#endif