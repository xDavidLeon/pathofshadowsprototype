#include "system_automat.h"
#include "entity.h"
#include <map>
#include "component.h"
#include "entity_manager.h"
#include "component_automat.h"
#include "world.h"
#include "aut_player.h"

AutomatSystem::AutomatSystem(void)
{
}


AutomatSystem::~AutomatSystem(void)
{
}

void AutomatSystem::update(float delta)
{
	//Se obtienen todas las entidades con componente automata
	std::map<Entity*,Component*>* entitiesWithAutomat = EntityManager::get().getAllEntitiesPosessingComponent<AutomatComponent>();

	//Se llama el update de los componentes automata
	std::map<Entity*,Component*>::iterator iter;
	for (iter = entitiesWithAutomat->begin(); iter != entitiesWithAutomat->end(); ++iter)
	{
		if(!iter->first->enabled) continue;
		iter->second->update(delta);
	}

	//if(CIOStatus::instance()->becomesPressed_key('N'))
	//{
	//	lockPlayer(true);
	//}

	//if(CIOStatus::instance()->becomesPressed_key('M'))
	//{
	//	lockPlayer(false);
	//}

}

void AutomatSystem::lockPlayer(bool lock)
{
	AutomatComponent* aC = EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer());

	if(lock)
	{
		((AUTPlayer*)aC->getAutomat())->stopMovement();
		((AUTPlayer*)aC->getAutomat())->changeState("idleBasic");
	}

	((AUTPlayer*)aC->getAutomat())->_lockedInPlace = lock;
}
