#ifndef _BT_PATROL
#define _BT_PATROL

#include "behaviour_tree.h"
#include "component_enemy_data.h"
#include <deque>
#include "bt_common.h"

class BTPatrol : public BTCommon
{
	std::vector<btVector3> _wayPoints;
	std::deque<btVector3> _pathToWP;
	unsigned _currentWP;
	bool _goingToFirstWP;
	btVector3* _firstWP;
	CounterClock _attackClockTemp, _idleClock;
	float _idleTimeFixed;
	float _idleWalkFixed;
	int _idleTimeVariable;
	bool _doingIdle;

public:
	BTPatrol(Entity* entity);

	void create();
	void render();
	void init();

	void addWayPoint(const btVector3& wp);

	//Conditions (specific for bt patrol)
	bool checkIfNotPatroling();
	bool checkIdle();
	bool checkGoingToFirstWP();
	bool checkcurrentWPReached();

	//Actions (specific for bt patrol)
	int generatePathToWP();
	int idle();
	int goToFirstWP();
	int changeCurrentWP();
	int goToCurrentWP();
};

#endif