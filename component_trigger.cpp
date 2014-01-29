#include "component_trigger.h"
#include "component_transform.h"
#include "system_trigger.h"
#include "logic_manager.h"
#include "entity_manager.h"

TriggerComponent::TriggerComponent(Entity* e) : Component(e)
{
	_radiusSq = -1;
	_playerInNow = _playerInBefore = false;
	_command = "";
}


TriggerComponent::~TriggerComponent(void)
{
}

void TriggerComponent::setType(const std::string &type_str)
{
	if(type_str == "ONENTER") _type = triggerType::ONENTER;
	else if(type_str == "ONEXIT") _type = triggerType::ONEXIT;
	else if(type_str == "ONFIRSTENTER") _type = triggerType::ONFIRSTENTER;
	else if(type_str == "ONFIRSTEXIT") _type = triggerType::ONFIRSTEXIT;
}

void TriggerComponent::update(float delta)
{
	//Comprobar si el player esta dentro
	_playerInBefore = _playerInNow;
	const btVector3 triggerPos = EntityManager::get().getComponent<TransformComponent>(entity)->getPosition();
	_playerInNow = EntityManager::get().getPlayerPos().distance2(triggerPos) < _radiusSq;

	//Comprobar si el trigger se dispara en este update
	switch(_type)
	{
		case triggerType::ONENTER:
			if(!_playerInBefore && _playerInNow)
			{
				LogicManager::get().runScript(_command.c_str());
			}
			break;
		case triggerType::ONEXIT:
			if(_playerInBefore && !_playerInNow)
			{
				LogicManager::get().runScript(_command.c_str());
			}
			break;
		case triggerType::ONFIRSTENTER:
			if(!_playerInBefore && _playerInNow)
			{
				//if(_command == "trigger001()") return;
				LogicManager::get().runScript(_command.c_str());

				//mandar trigger a borrar
				TriggerSystem::get().addTriggerToRemove(entity);
			}
			break;
		case triggerType::ONFIRSTEXIT:
			if(_playerInBefore && !_playerInNow)
			{
				LogicManager::get().runScript(_command.c_str());

				//mandar trigger a borrar
				TriggerSystem::get().addTriggerToRemove(entity);
			}
			break;
	}
}
