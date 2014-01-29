#ifndef SYS_TRIGGER
#define SYS_TRIGGER

#include "counter_clock.h"
#include "entity.h"
#include <vector>

class TriggerSystem
{
	TriggerSystem(void);
	~TriggerSystem(void);
	CyclicCounterClock _updateClock;
	std::vector<Entity*> _toRemove;

	void removeDeadTriggers();

public:
	static TriggerSystem &get()
	{
		static TriggerSystem ts;
		return ts;
	}

	void update(float delta);
	void addTriggerToRemove(Entity* trigger_entity);
	void release();
};

#endif