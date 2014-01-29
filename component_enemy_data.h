#ifndef ENEMY_DATA_COMP
#define ENEMY_DATA_COMP

#include "component.h"
#include "component_transform.h"
#include "counter_clock.h"
#include <vector>
#include "entity.h"
#include <deque>
#include "component_shadow_actions.h"
#include "system_sound.h"

class BehaviourTree;

static const float minAngleToRun = 1.5708f;
static const btVector3 eyeOffset_v = btVector3(0,1.02f,0);

enum attentionDegrees{
	NORMAL,
	CAUTION,
	PERMANENT_CAUTION, //Una vez se ha visto al player no se vuelve a normal
	ALERT,
	PANIC
};

class EnemyDataComponent : public Component
{
	//Para crashNTurn
	std::vector<float> _CToffsets;
	int _CToffset;
	int _currentLookAtId;
	CounterClock _crashAndTurningCounter;

public:
	EnemyDataComponent(Entity* e, TransformComponent* tc);
	~EnemyDataComponent(void);

	void init();
	
	float _delta; //TO DEPRECATE mucho

	bool _damageTaken, _searching;

	float _life;
	bool _silent_kill;
	attentionDegrees _attentionDegree;
	TransformComponent* _transformC;
	unsigned _dbgColor;
	//<vision>
	float _visionPercent;
	float _halfFov, _halfFovPeriferial;
	float _periferialDistSq;
	float _visionThreshold; //Factor a partir del cual el enemigo empieza a sospechar
	float _visionDistSq;
	float _visionDistSq_normal, _visionFactor_caution, _visionFactor_alert;
	float _visionDistSq_playerInLight, _visionDistSq_playerInShadows;
	//</vision>
	float _rotateVel;
	float _currentVel;
	float _walkVel, _jogVel, _runVel;
	float _angleWithPlayer;
	bool  _playerSeen;
	bool _alerting, _alertThrown;
	bool _lookedAtSoundSource;
	float _hearDistSq;
	float _warDistMh;
	btVector3 _eyePos;
	btVector3 _lookAt;
	std::vector<btVector3> _lookAts;
	int _lastPlayerNode;
	std::deque<btVector3> _path;
	btVector3 _soundPlace;

	CounterClock _lookAtClock, _forgetClock;
	float _lookAtTime, _forgetTime;
	float _corpseLookTime;

	bool _crashNTurning;

	playerVisibility _enemyTexture;

	void advance(const btVector3& v_delta);
	void setAttentionDegree(attentionDegrees a_d);
	void setLookAts(float dist);
	void chooseLookAt();
	void silentKill();
	void dead();
	void goToNextTrackPoint(float delta, std::deque<btVector3>& path);
	bool crashNTurn(std::deque<btVector3>& path);

	void renderPath(unsigned color, float offset);

	//SoundSystem::SoundInfo * _soundArmor;
};

#endif
