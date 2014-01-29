#include "camera_controller.h"
#include "component_transform.h"
#include "entity_manager.h"

CameraController::CameraController()
{
}

CameraController::CameraController( TCamera* camera)
{
	_camera = camera;
}


CameraController::~CameraController()
{
}

void CameraController::setTargetEntity(Entity* e)
{
	_targetEntity = e;
	TransformComponent * tComponent = EntityManager::get().getComponent<TransformComponent>(e);
	_targetTransform = tComponent->transform;
}
