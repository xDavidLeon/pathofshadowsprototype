#ifndef BT_SYSTEM_H
#define BT_SYSTEM_H

#include <map>
#include <set>
#include "btBulletDynamicsCommon.h"
#include "counter_clock.h"

class Entity;
class BehaviourTree;

class BTSystem
{
	std::map<Entity*, btVector3*> _sbbWarnings;
	int _playerNode;
	std::set<Entity*> _deadEnemies;
	std::set<Entity*> _toRemove;
	std::set<BehaviourTree*> _BTtoRemove;
	CyclicCounterClock _visibilityClock;
	bool _panicButtonPressed, _panicButtonPressedBefore;

	void updateTexture(Entity* entity, float delta);

	BTSystem(void);
	~BTSystem(void);
public:
	static BTSystem & get()
	{
		static BTSystem singleton;
		return singleton;
	}

	void update(float delta);
	void render();

	void addSbbWarning(Entity* entity, const btVector3& pos);
	void removeSbbWarning(Entity* entity);
	const std::map<Entity*, btVector3*>& getSbbWarnings() const{ return _sbbWarnings; }
	int getplayerNode() const { return _playerNode; }
	const std::set<Entity*>& getDeadEnemies() const { return _deadEnemies; }
	void addDeadEnemy(Entity* corpse);
	void eraseDeadEnemy(Entity* corpse);
	void addEntityToRemove(Entity* toRemove);
	void destroyEnemy(Entity* enemy);
	void removeEntities();
	void addBTToRemove(BehaviourTree* toRemove);
	void removeBTs();
	void release();
	void pressPanicButton(){ _panicButtonPressed = true; }
	bool isPanicButtonPressed(){ return _panicButtonPressed; }
};

#endif
