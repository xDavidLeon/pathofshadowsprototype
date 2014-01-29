#ifndef PLAYER_CONTROLLER_COMP
#define PLAYER_CONTROLLER_COMP

#include "component.h"
#include <map>
#include <string>
#include "btBulletDynamicsCommon.h"

const static float decoyNoiseSq = 144.0f; //12 "m"
const static float runNoiseSq = 4.0f; //2 "m"

class PlayerControllerComponent : public Component
{
	std::map<std::string, btTransform> _respawns;

public:
	PlayerControllerComponent(Entity* e);
	~PlayerControllerComponent(void);
	void init();

	//booleanos para ir controlando cuando usar las mecanicas
	bool _canRecharge;

	float _life;
	float _costCreateShadow, _costGrowShadow, _costTeleport, _costBlended;
	float _rechargeRatio;
	float _damageInLight;
	float _noiseDistSq;

	bool hasLifeForCreateShadow() const;
	bool hasLifeForGrowShadow() const;
	bool hasLifeForTeleport() const;
	bool hasLifeForBlend() const;
	void rechargeLife();
	void receiveDamage(float damage);
	void damageInLight();
	float getLife() { return _life; };
	void setLife(float new_life){ _life = new_life; }

	void addRespawn(const char* id, const btTransform& T);
	const btTransform* getRespawn(const char* id);
};

#endif
