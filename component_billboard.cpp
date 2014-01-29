#include "component_billboard.h"
#include <cassert>
#include "system_camera.h"
#include "d3ddefs.h"
#include "iostatus.h"

BillboardComponent::BillboardComponent(Entity* entity, TransformComponent * t, ModelComponent * m) : UniqueComponent(entity)
{
	transform = t;
	model = m;
	target_alpha = 1.0f;
	velocity_multiplier = 1.0f;
}

BillboardComponent::~BillboardComponent(void)
{
}

void BillboardComponent::update(float delta)
{
	btVector3 cam_front;
	CameraSystem::get().getCurrentCamera().getFront(cam_front);
	transform->lookAtBillboard(cam_front, btVector3(0,1,0));

	if (model->diffuseColor.w < target_alpha) 
	{
		model->diffuseColor.w += delta * velocity_multiplier;
		if (model->diffuseColor.w > target_alpha) model->diffuseColor.w = target_alpha;
	}
	else if (model->diffuseColor.w > target_alpha) 
	{
		model->diffuseColor.w -= delta * velocity_multiplier;
		if (model->diffuseColor.w < target_alpha) model->diffuseColor.w = target_alpha;
	}

}

void BillboardComponent::setTargetAlpha(float new_alpha, float velocity_factor)
{
	target_alpha = new_alpha;
	velocity_multiplier = velocity_factor;
}
