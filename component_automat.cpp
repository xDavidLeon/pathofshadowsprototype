#include "component_automat.h"
#include "aut_player.h"
#include "world.h"

AutomatComponent::AutomatComponent(Entity* entity, automatTypes automatType) : Component(entity)
{
	_automatType = automatType;
	_automat = NULL;
	
	switch(_automatType)
	{
		case(automatTypes::PLAYER):
			if(_automat) delete _automat;
			_automat = new AUTPlayer(World::instance()->getPlayer());
			_automat->init();
			break;
	}
}

void AutomatComponent::init()
{
	_automat->reset();
}

AutomatComponent::~AutomatComponent(void)
{
	delete _automat;
}

void AutomatComponent::update(float delta)
{
	_automat->update(delta);
}

void AutomatComponent::render()
{
	_automat->render();
}