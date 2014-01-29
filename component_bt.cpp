#include "component_bt.h"
#include "bt_gatekeeper.h"
#include "bt_patrol.h"
#include "system_bt.h";
#include "bt_crow.h"
#include "bt_goddess.h"

BTComponent::BTComponent(Entity* e, btTypes btType) : Component(e)
{
	_entity = e;
	_btType = btType;
	_bt = NULL;

	switch(_btType)
	{
		case btTypes::GATEKEEPER:
			if(_bt) delete _bt;
			_bt = new BTGatekeeper(_entity);
			break;
		case btTypes::PATROLER:
			if(_bt) delete _bt;
			_bt = new BTPatrol(_entity);
			break;
		case btTypes::CROW:
			if(_bt) delete _bt;
			_bt = new BTCrow(_entity);
			break;
		case btTypes::GODDESS:
			if(_bt) delete _bt;
			_bt = new BTGoddess(_entity);
			break;
	}
}

void BTComponent::init()
{
	switch(_btType)
	{
		case btTypes::GATEKEEPER:
			if(_bt){ _bt->reset(); _bt->init();}
			break;
		case btTypes::PATROLER:
			if(_bt){ _bt->reset(); _bt->init();}
			break;
		case btTypes::CROW:
			if(_bt){ _bt->init();}
			break;
	}
}

BTComponent::~BTComponent(void)
{
	delete _bt;
}

void BTComponent::update(float delta)
{
	_bt->recalc(delta);
}

void BTComponent::render()
{
	_bt->render();
}

void BTComponent::changeBT(BehaviourTree* newBT)
{
	if(_bt) BTSystem::get().addBTToRemove(_bt);
	_bt = newBT;
}
