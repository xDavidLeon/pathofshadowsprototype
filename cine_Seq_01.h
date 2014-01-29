#ifndef CINE_SEQ_01
#define CINE_SEQ_01

#include "behaviour_tree.h"
#include "entity.h"
#include <deque>
#include "component_transform.h"
#include "counter_clock.h"
#include "texture_manager.h"

class CineSeq01 : public BehaviourTree
{
	CounterClock _clock;
	Entity* _shadowEffectBirth;
	bool _started, _logoFirstEnabled, _playerBorn;

public:
	CineSeq01(Entity* entity);
	~CineSeq01();

	void create();
	void init(){};
	void render();

	//Actions
	int cineCam1();
	int playerBirth();
	int die();
};

#endif