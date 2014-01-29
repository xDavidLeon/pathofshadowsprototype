#ifndef BLL_COMP
#define BLL_COMP

#include "component_unique.h"
#include "component_transform.h"
#include "component_model.h"

class BillboardComponent : public UniqueComponent
{

public:
	BillboardComponent(Entity* cam_entity, TransformComponent * t, ModelComponent * m);
	~BillboardComponent(void);

	void update(float delta);

	void setTargetAlpha(float new_alpha, float velocity_factor);

private:
	TransformComponent * transform;
	ModelComponent * model;
	
	float		target_alpha;
	float		velocity_multiplier;
};

#endif
