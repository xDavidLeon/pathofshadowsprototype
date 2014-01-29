#include "system_trigger.h"
#include "component_trigger.h"
#include "entity_manager.h"
#include "system_debug.h"
#include "vision_interface.h"

TriggerSystem::TriggerSystem(void)
{
	_updateClock.setTarget(0.7f);
	//_updateClock.setCount(0.5f); //Para que al empezar compruebe
}


TriggerSystem::~TriggerSystem(void)
{
}

void TriggerSystem::update(float delta)
{
	if(!_updateClock.count(delta)) return;

	//Para evitar (entre otras cositas) que te puedas pasar el nivel si hay algún enemigo en alert
	if(VisionInterface::get().thereIsAlert()) return;

	removeDeadTriggers();

	std::map<Entity*, Component*> *triggers = EntityManager::get().getAllEntitiesPosessingComponent<TriggerComponent>();
	if(triggers == NULL) return;

	std::map<Entity*, Component*>::iterator trigger;
	for(trigger=triggers->begin(); trigger!=triggers->end(); trigger++)
	{
		if(trigger->first->enabled) ((TriggerComponent*)trigger->second)->update(delta);
	}
}

void TriggerSystem::removeDeadTriggers()
{
	std::vector<Entity*>::iterator toRemove;
	while(_toRemove.size())
	{
		toRemove = _toRemove.begin();
		EntityManager::get().removeEntity(*toRemove);
		_toRemove.erase(toRemove);
	}
}

void TriggerSystem::addTriggerToRemove(Entity* trigger_entity)
{
	_toRemove.push_back(trigger_entity);
}

void TriggerSystem::release()
{
	_toRemove.clear();
}
