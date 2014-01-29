#ifndef BT_COMMON
#define BT_COMMON

#include "entity.h"
#include "component_enemy_data.h"
#include "behaviour_tree.h"
#include <set>
#include <deque>

class Entity;

class BTCommon : public BehaviourTree
{
protected:
	CounterClock _attackClockTemp, _lookAllyTemp, _panicClock, _lookChaserAlly;

	bool _trackingPoint, _trackingPlayer, _trackingCorpse, _unsheathed, _attacking, _stoppedSearching, _turning;
	btVector3 _targetCorpse, _pointToSee;

	Entity* _corpseE;

	EnemyDataComponent* _eD;
	std::set<Entity*> _seenCorpses; //para guardar los cadáveres en cuanto se ven.

	Entity* _chaserAlly;

public:
	BTCommon(Entity* entity);
	~BTCommon();

	virtual void create(const string&){};
	void init();

	bool _chasePlayerNow; //bool guarro para forzar a "ver" al player

	//Conditions
	bool checkIfHasToPanic();
	bool checkDamageReceived();
	bool checkWarDistance();
	bool checkPlayerViewed();
	bool checkIfWantToAlert();		//"Aviso antes o voy directamente??"
	bool checkAlertNotThrown();
	bool checkSheathed();
	bool checkTrackingTarget();
	bool checkHasTrackPoints();
	bool checkAllyChasingPlayer();
	bool checkCorpseSeen();
	bool checkIfFirstCorpse();
	bool checkNearCorpse();
	bool checkSeeingCorpse();
	bool checkWarningInBlackboard();
	bool checkWantToGo();
	bool checkLoudNoiseHeard();
	bool checkPlayerPartiallyViewed();
	bool checkSearching();
	bool checkStoppedSearching();
	bool checkHasLookAt();

	//Actions
	int fall();
	int beScared();
	int standUp();
	int takeDamage();
	int die();
	int attack(); // futura cinematica de "pillado" 
	int alertAllies();
	int unsheathe();
	int chasePlayer();
	int trackNextPoint();
	int stopTracking();
	int lookAtChaser();
	int goWhereAllyGoes();
	int lookAtCorpse();
	int goToCorpse();
	int rememberCorpse();
	int lookToAlly();
	int unsheatheIfHasTo();
	int goWithAlly();
	int ignoreWarning();
	int lookAtSoundSource();
	int goToSoundSource();
	int lookForPlayer();
	int stopSearching();
	int sheathe();
	int lookAt();
	int chooseLookAt();

	//otras
	void pathfind(const btVector3& point, std::deque<btVector3>& path);
	EnemyDataComponent* getEnemyDataComponent() { return _eD; };
	void setUnsheathed( bool state ) { _unsheathed = state; };
	bool getUnsheathed( ) { return _unsheathed; };
	void decrementVisionPercent();
	void incrementVisionPercent(float factorBase);
};

#endif