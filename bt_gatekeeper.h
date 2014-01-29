#ifndef _BT_GATEKEEPER
#define _BT_GATEKEEPER

#include "behaviour_tree.h"
#include <deque>
#include "bt_common.h"
#include "counter_clock.h"

class BTGatekeeper : public BTCommon
{
	std::deque<btVector3> _pathToGK;
	bool _goingToGKPlace;
	btVector3 _GKPlace;
	btVector3 _GKLookAt;
	CounterClock _secondIdle;
	bool _secondIdleEnabled;
	Entity* _allyToTalk;
	bool _thereWasAllyToTalk;
	bool _talkingToLeft;

public:
	BTGatekeeper(Entity* entity);

	void create();
	void render();

	void setGKPlace(const btVector3& place);
	void setGKLookAt(const btVector3& lookAt);
	const btVector3& getGKPlace() const { return _GKPlace; }
	const btVector3& getGKLookAt() const { return _GKLookAt; }

	void setAllyToTalk(Entity* toTalk, bool setTheOther=true, bool stopTalkingWithTheOther=false);

	//Conditions (specific for gatekeeper)
	bool checkIfNotGatekeeping();
	bool checkgoingToGKPlace();
	bool checkLookingAtGK();
	bool checkTalk();
	bool checkTalkLeft();
	bool checkIdleTimeOut();

	//Actions (specific for gatekeeper)
	int generatePathToGK();
	int goToGKPlace();
	int lookAtGK();
	int talkLeft(); //Las acciones de talk diferentes solo sirven para la animacion
	int talkRight();
	int idle2();
	int idle();
};

#endif