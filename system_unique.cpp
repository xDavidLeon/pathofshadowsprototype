#include "system_unique.h"
#include "component_unique.h"
#include "component_billboard.h"
#include "entity_manager.h"

UniqueSystem::UniqueSystem(void)
{
}

UniqueSystem::~UniqueSystem(void)
{
}

void UniqueSystem::update(float delta)
{
	std::map<Entity*, Component*> *uniques = EntityManager::get().getAllEntitiesPosessingComponent<BillboardComponent>();
	if(uniques != NULL)
	{
		std::map<Entity*, Component*>::iterator u;
		for(u=uniques->begin(); u!=uniques->end(); u++)
		{
			((BillboardComponent*)u->second)->update(delta);
		}
	}

	std::map<Entity*, bool>::iterator iter = _tutorial_msgs.begin();
	while (iter != _tutorial_msgs.end())
	{
		if(!iter->first->enabled)
		{
			iter++;
			continue;
		}
		ModelComponent * m = EntityManager::get().getComponent<ModelComponent>(iter->first);
		if (iter->second == true) 
		{
			m->diffuseColor.w += delta;
			if (m->diffuseColor.w > 1.0f) m->diffuseColor.w = 1.0f;
		}
		else 
		{
			m->diffuseColor.w -= delta;
			if(m->diffuseColor.w < 0.0f)
			{
				iter->first->enabled = false;
			}
		}

		iter++;
	}

}

void UniqueSystem::addTutorial(Entity * tut)
{
	_tutorial_msgs.insert(std::pair<Entity*,bool>(tut,false));
}

void UniqueSystem::removeTutorial(Entity * tut)
{
	_tutorial_msgs.erase(tut);
}

void UniqueSystem::tutorialAppear(Entity * tut, bool appear)
{
	if(!tut) return;
	tut->enabled = true;
	_tutorial_msgs.at(tut) = appear;
}

void UniqueSystem::release()
{
	_tutorial_msgs.clear();
}
