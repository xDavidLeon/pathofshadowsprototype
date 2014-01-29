#include "automat.h"
#include "globals.h"

Automat::Automat(Entity* entity)
{
	_entity = entity;
}

void Automat::update(float delta)
{
	// this is a trusted jump as we've tested for coherence in ChangeState
	(this->*_statemap[_state])(delta);
}

void Automat::changeState(const std::string& newstate)
{
	// try to find a state with the suitable name
	if (_statemap.find(newstate) == _statemap.end())
	{
		// the state we wish to jump to does not exist. we abort
		assert(0);
	}

	_state=newstate;
	_clock = 0.0f;
}

void Automat::addState(std::string name, statehandler sh)
{
	// try to find a state with the suitable name
	if (_statemap.find(name) != _statemap.end())
	{
		// the state we wish to jump to does exist. we abort
		assert(0 || fatal("Automat: Intento de saltar a estado inexistente"));
	}

	_statemap[name]=sh;
}

const string& Automat::getState() const
{
	return _state;
}

Entity* Automat::getEntity()
{
	return _entity;
}

void Automat::setEntity( Entity* entity)
{
	_entity = entity;
}
