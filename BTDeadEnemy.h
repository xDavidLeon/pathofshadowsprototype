#ifndef BT_DEAD_ENEMY
#define BT_DEAD_ENEMY

#include "behaviour_tree.h"
#include "counter_clock.h"

class BTDeadEnemy : public BehaviourTree
{
	D3DXVECTOR3 _posDX;
	btVector3	_posBT;
	CounterClock _immerseCount;

public:
	BTDeadEnemy(Entity* entity, const btVector3& pos);
	~BTDeadEnemy(void);

	void init(){};

	void create();
	void render();

	//Conditions
	bool checkInMagicShadow();

	//Actions
	int immerse();
	int autoDelete();
	int idle();

};

#endif
