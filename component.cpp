#include "component.h"
#include "entity.h"
Component::Component(Entity* e)
{
	enabled = true;
	entity = e;
}