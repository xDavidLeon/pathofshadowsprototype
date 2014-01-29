#ifndef UNQ_SYS
#define UNQ_SYS

#include "entity.h"
#include <map>

class UniqueSystem
{
	UniqueSystem(void);
	~UniqueSystem(void);

	std::map<Entity*, bool> _tutorial_msgs;

public:
	static UniqueSystem & get()
	{
		static UniqueSystem cs;
		return cs;
	}

	//void render(void);
	void update(float delta);
	void addTutorial(Entity * tut);
	void removeTutorial(Entity * tut);

	void tutorialAppear(Entity * tut, bool appear);
	void release();
};

#endif
