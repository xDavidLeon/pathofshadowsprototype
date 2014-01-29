#ifndef BTCROW
#define BTCROW

#include "behaviour_tree.h"
#include "entity.h"
#include <deque>
#include "component_transform.h"
#include "counter_clock.h"
class ParticleEffectComponent;

class BTCrow : public BehaviourTree
{
	std::deque<btVector3> _pathToEnd;
	TransformComponent* _transformC;
	CounterClock _clock;
	float _crowVel, _catmullIndex;
	btVector3 _posBefore, _initFront, _initPos;
	SoundSystem::SoundInfo * _sound;
	btTransform vOffset_bt;
	ParticleEffectComponent * _particleCrow;
	bool generatePathToEnd();
	void createTransform(const btVector3& front_v, const btVector3& pos); 

public:
	BTCrow(Entity* entity);
	~BTCrow();

	virtual void create(const string&){};
	void init();
	void create();
	void render();

	//Actions
	int disappear();
	int trackNextPoint();
	int rotate();
	int appear();
};

#endif
