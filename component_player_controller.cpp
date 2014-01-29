#include "component_player_controller.h"

PlayerControllerComponent::PlayerControllerComponent(Entity * e) : Component(e)
{
	_costCreateShadow = 5.0f; //DEPRECATED
	_costGrowShadow = 0.4f;
	_costTeleport = 50.0f;
	_costBlended = 0.15f;
	_rechargeRatio = 0.2f;
	_damageInLight = 0.4f;

	_canRecharge = true;

	init();
}

void PlayerControllerComponent::init()
{
	_life = 100.0f;
	_noiseDistSq = 0.0f;
}

PlayerControllerComponent::~PlayerControllerComponent(void)
{
}

bool PlayerControllerComponent::hasLifeForCreateShadow() const
{
	return _life >_costCreateShadow;
}

bool PlayerControllerComponent::hasLifeForGrowShadow() const
{
	return _life > _costGrowShadow;
}

bool PlayerControllerComponent::hasLifeForTeleport() const
{
	return _life > _costTeleport;
}

bool PlayerControllerComponent::hasLifeForBlend() const
{
	return _life > _costBlended;
}

void PlayerControllerComponent::rechargeLife()
{
	if(!_canRecharge) return;
	_life += _rechargeRatio;
	if(_life > 100.0f) _life = 100.0f;
}

void PlayerControllerComponent::receiveDamage(float damage)
{
	_life -= damage;
	if(_life < 0.0f) _life = 0.0f;
}

void PlayerControllerComponent::damageInLight()
{
	if(_life - _damageInLight > 0.0f)
		receiveDamage(_damageInLight);
}

void PlayerControllerComponent::addRespawn(const char* id, const btTransform& T)
{
	_respawns.insert(std::pair<std::string, btTransform>(id, T));
}

const btTransform* PlayerControllerComponent::getRespawn(const char* id)
{
	if(_respawns.find(id) == _respawns.end()) return NULL;
	else return &_respawns.at(id);
}

