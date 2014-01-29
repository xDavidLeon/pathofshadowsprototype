#ifndef UNQ_COMP
#define UNQ_COMP

#include "component.h"

class UniqueComponent : public Component
{

public:
	UniqueComponent(Entity* cam_entity);
	~UniqueComponent(void);

	virtual void update(float delta){};
};

#endif
