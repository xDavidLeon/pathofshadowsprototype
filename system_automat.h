#ifndef AUTOMAT_SYSTEM_H
#define AUTOMAT_SYSTEM_H

#include "counter_clock.h"
class AutomatSystem
{
public:
	static AutomatSystem & get() {
		static AutomatSystem singleton;
		return singleton;
	}	
	void update(float delta);
	void lockPlayer(bool lock);

private:
	AutomatSystem(void);
	~AutomatSystem(void);
};

#endif
