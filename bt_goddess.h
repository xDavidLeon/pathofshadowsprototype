#ifndef BT_GODDESS
#define BT_GODDESS

#include "behaviour_tree.h"
#include "entity.h"
#include "component_transform.h"
#include <deque>
#include <string>

class BTGoddess : public BehaviourTree
{
	TransformComponent *_crowTransformC, *_goddessTransformC;
	Entity *_goddessModel, *_crowModel;
	bool _hasToGo, _isBorn, _hasToChangePlace, _hasToBeBorn, _hasToGivePowers;
	float _rotVel;
	std::deque<std::string> _respawns;
	btVector3 _lookAt;

public:
	BTGoddess(Entity* entity);
	~BTGoddess(void);

	void create();
	void render();
	void init(){};

	void setEntitites(Entity* crow, Entity* goddess);
	void setLookat(const btVector3& la);

	void goddessBorn();
	void changePlace();
	void givePowersToPlayer();
	void hasToGo();


	//Condiciones
	bool checkHasToGo();
	bool checkIsBorn();
	bool checkHasToGivePowers();
	bool checkHasToChangePlace();
	bool checkHasToBeBorn();

	//Acciones
	int poof();
	int givePowers();
	int dissappear();
	int appear();
	int idleGoddess();
	int flyToPlace();
	int beBorn();
	int idleCrow();

	Entity* getCrowEntity() { return _crowModel; };
	Entity* getGoddessEntity() { return _goddessModel; };
};

#endif